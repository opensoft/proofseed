/* Copyright 2018, OpenSoft Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *     * Neither the name of OpenSoft Inc. nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: denis.kormalev@opensoftdev.com (Denis Kormalev)
 *
 */
#include "proofseed/tasks.h"

#include "proofseed/spinlock.h"

#include <QCoreApplication>
#include <QMutex>
#include <QString>
#include <QThread>
#include <QWaitCondition>

#include <list>
#include <map>
#include <set>
#include <vector>

static const qint32 INTENSIVE_CAPACITY = qMax(QThread::idealThreadCount(), 1);
constexpr qint32 CUSTOM_CAPACITY = 16;
constexpr qint32 DEFAULT_TOTAL_CAPACITY = 64;

namespace Proof {
namespace tasks {

struct TaskInfo
{
    TaskInfo(std::function<void()> &&_task, RestrictionType _type, const QString &_restrictor)
        : task(std::move(_task)), restrictionType(_type), restrictor(_restrictor)
    {}
    std::function<void()> task;
    // having multiple restrictors is technically possible and in some rare cases can help a lot,
    // but due to possibility for task never run they are not allowed
    RestrictionType restrictionType;
    QString restrictor;
};

class Worker;
class TasksDispatcherPrivate
{
    Q_DECLARE_PUBLIC(TasksDispatcher)
    TasksDispatcher *q_ptr = nullptr;

public:
    void taskFinished(qint32 workerId, const TaskInfo &taskInfo);

private:
    void schedule(qint32 forcedWorkerId = -1); //should be always called under mainLock
    bool tryTaskScheduling(const TaskInfo &task, qint32 workerId);

    std::vector<Worker *> workers;
    std::set<QThread *> poolThreads;
    std::set<qint32> waitingWorkers;
    std::set<qint32> waitingBoundWorkers;
    std::list<TaskInfo> tasks;
    std::map<QString, qint32> workerBindings;
    std::map<qint32, int> boundWorkers;
    std::map<RestrictionType, std::map<QString, qint32>> restrictorsUsage;
    SpinLock mainLock;
    std::map<QString, qint32> customRestrictorsCapacity;
    qint32 capacity = DEFAULT_TOTAL_CAPACITY;

    static thread_local bool currentEventLoopStarted;
    static thread_local QSharedPointer<QEventLoop> signalWaitersEventLoop;
};
thread_local bool TasksDispatcherPrivate::currentEventLoopStarted = false;
thread_local QSharedPointer<QEventLoop> TasksDispatcherPrivate::signalWaitersEventLoop;

class Worker : public QThread
{
    Q_OBJECT
public:
    explicit Worker(qint32 id);
    void setNextTask(const TaskInfo &task);
    void poisonPill();

protected:
    void run() override;

private:
    qint32 id = 0;
    std::atomic_bool poisoned{false};
    std::experimental::optional<TaskInfo> nextTask;
    QWaitCondition waiter;
    QMutex taskLock;
};

} // namespace tasks
} // namespace Proof

using namespace Proof::tasks;

TasksDispatcher::TasksDispatcher() : d_ptr(new TasksDispatcherPrivate)
{
    d_ptr->q_ptr = this;
}

TasksDispatcher::~TasksDispatcher()
{
    d_ptr->mainLock.lock();
    for (auto worker : d_ptr->workers)
        worker->poisonPill();
    for (auto worker : d_ptr->workers)
        worker->wait(100);
    for (auto worker : d_ptr->workers) {
        if (worker->isRunning()) {
            worker->terminate();
            worker->wait(100);
        }
        delete worker;
    }
    d_ptr->workers.clear();
    d_ptr->mainLock.unlock();
}

TasksDispatcher *TasksDispatcher::instance()
{
    static TasksDispatcher tasksDispatcher;
    return &tasksDispatcher;
}

qint32 TasksDispatcher::capacity() const
{
    return d_ptr->capacity;
}

qint32 TasksDispatcher::restrictorCapacity(RestrictionType restrictionType, const QString &restrictor) const
{
    if (restrictionType == RestrictionType::ThreadBound)
        return 1;
    if (restrictionType == RestrictionType::Intensive)
        return INTENSIVE_CAPACITY;
    if (restrictor.isEmpty())
        return capacity();
    d_ptr->mainLock.lock();
    auto resultIt = d_ptr->customRestrictorsCapacity.find(restrictor);
    qint32 result = d_ptr->customRestrictorsCapacity.cend() == resultIt ? CUSTOM_CAPACITY : resultIt->second;
    d_ptr->mainLock.unlock();
    return result;
}

void TasksDispatcher::setCapacity(qint32 capacity)
{
    capacity = qMin(1, capacity);
    d_ptr->mainLock.lock();
    if (static_cast<qint32>(d_ptr->workers.size()) <= capacity)
        d_ptr->capacity = capacity;
    d_ptr->mainLock.unlock();
}

void TasksDispatcher::addCustomRestrictor(const QString &restrictor, qint32 capacity)
{
    capacity = qBound(1, capacity, d_ptr->capacity);
    d_ptr->mainLock.lock();
    d_ptr->customRestrictorsCapacity[restrictor] = capacity;
    d_ptr->mainLock.unlock();
}

void TasksDispatcher::fireSignalWaiters() noexcept
{
    Q_ASSERT_X(d_ptr->poolThreads.count(QThread::currentThread()), "fireSignalWaiters",
               "signal waiters can't be used outside of tasks thread pool");
    qApp->processEvents();
    if (!d_ptr->signalWaitersEventLoop)
        return;
    d_ptr->currentEventLoopStarted = true;
    d_ptr->signalWaitersEventLoop->exec();
    clearEventLoop();
}

void TasksDispatcher::insertTaskInfo(std::function<void()> &&wrappedTask, RestrictionType restrictionType,
                                     const QString &restrictor) noexcept
{
    d_ptr->mainLock.lock();
    //We mimic all intensive tasks as under single restrictor
    d_ptr->tasks.emplace_back(std::move(wrappedTask), restrictionType,
                              restrictionType == RestrictionType::Intensive ? QStringLiteral("_") : restrictor);
    d_ptr->schedule();
    d_ptr->mainLock.unlock();
}

void TasksDispatcher::addSignalWaiterPrivate(std::function<void(const QSharedPointer<QEventLoop> &)> &&connector) noexcept
{
    Q_ASSERT_X(d_ptr->poolThreads.count(QThread::currentThread()), "addSignalWaiterPrivate",
               "signal waiters can't be used outside of tasks thread pool");
    if (!d_ptr->signalWaitersEventLoop) {
        d_ptr->currentEventLoopStarted = false;
        d_ptr->signalWaitersEventLoop.reset(new QEventLoop);
    }
    connector(d_ptr->signalWaitersEventLoop);
}

bool TasksDispatcher::eventLoopStarted() const noexcept
{
    return d_ptr->currentEventLoopStarted;
}

void TasksDispatcher::clearEventLoop() noexcept
{
    d_ptr->signalWaitersEventLoop.clear();
    d_ptr->currentEventLoopStarted = false;
}

void TasksDispatcherPrivate::taskFinished(qint32 workerId, const TaskInfo &taskInfo)
{
    mainLock.lock();
    if (taskInfo.restrictionType != RestrictionType::ThreadBound && !taskInfo.restrictor.isEmpty()) {
        if (restrictorsUsage[taskInfo.restrictionType][taskInfo.restrictor] <= 1)
            restrictorsUsage[taskInfo.restrictionType].erase(taskInfo.restrictor);
        else
            --restrictorsUsage[taskInfo.restrictionType][taskInfo.restrictor];
    }
    if (boundWorkers.count(workerId)) {
        waitingBoundWorkers.insert(workerId);
        schedule();
    } else {
        waitingWorkers.insert(workerId);
        schedule(workerId);
    }
    mainLock.unlock();
}

void TasksDispatcherPrivate::schedule(qint32 forcedWorkerId)
{
    if (waitingWorkers.empty()) {
        qint32 workersSize = static_cast<qint32>(workers.size());
        if (workersSize < capacity) {
            waitingWorkers.insert(workersSize);
            auto worker = new Worker(workersSize);
            workers.push_back(worker);
            poolThreads.insert(worker);
            worker->start();
        } else {
            if (waitingBoundWorkers.empty())
                return;
            qint32 boundWorker = *waitingBoundWorkers.cbegin();
            waitingBoundWorkers.erase(boundWorker);
            waitingWorkers.insert(boundWorker);
        }
    }

    const qint32 precalculatedWorkerId = (forcedWorkerId >= 0 && waitingWorkers.count(forcedWorkerId))
                                             ? forcedWorkerId
                                             : *waitingWorkers.cbegin();
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        if (tryTaskScheduling(*it, precalculatedWorkerId)) {
            tasks.erase(it);
            break;
        }
    }
}

bool TasksDispatcherPrivate::tryTaskScheduling(const TaskInfo &task, qint32 workerId)
{
    if (task.restrictionType == RestrictionType::ThreadBound) {
        if (workerBindings.count(task.restrictor)) {
            workerId = workerBindings[task.restrictor];
            if (!waitingWorkers.count(workerId) && !waitingBoundWorkers.count(workerId))
                return false;
        } else {
            if (static_cast<qint32>(boundWorkers.size()) < capacity) {
                if (boundWorkers.count(workerId))
                    workerId = algorithms::findIf(waitingWorkers,
                                                  [this](qint32 x) -> bool { return !boundWorkers.count(x); }, -1);
            } else {
                auto minimizer = [this](qint32 acc, qint32 x) -> qint32 {
                    return acc < 0 || boundWorkers[acc] > boundWorkers[x] ? x : acc;
                };
                workerId = algorithms::reduce(waitingBoundWorkers, minimizer, -1);
                workerId = algorithms::reduce(waitingWorkers, minimizer, workerId);
            }
            if (workerId < 0)
                return false;
            ++boundWorkers[workerId];
            workerBindings[task.restrictor] = workerId;
        }
    } else if (!task.restrictor.isEmpty()) {
        qint32 capacityLeft = capacity;
        switch (task.restrictionType) {
        case RestrictionType::Intensive:
            capacityLeft = INTENSIVE_CAPACITY;
            break;
        case RestrictionType::Custom: {
            auto resultIt = customRestrictorsCapacity.find(task.restrictor);
            capacityLeft = customRestrictorsCapacity.cend() == resultIt ? CUSTOM_CAPACITY : resultIt->second;
            break;
        }
        default:
            break;
        }
        auto resultIt = restrictorsUsage[task.restrictionType].find(task.restrictor);
        capacityLeft -= restrictorsUsage[task.restrictionType].cend() == resultIt ? 0 : resultIt->second;
        if (capacityLeft <= 0)
            return false;
        ++restrictorsUsage[task.restrictionType][task.restrictor];
    }
    if (workerId < 0)
        return false;
    waitingWorkers.erase(workerId);
    waitingBoundWorkers.erase(workerId);
    workers[workerId]->setNextTask(task);
    return true;
}

Worker::Worker(qint32 id) : QThread(nullptr), id(id)
{}

void Worker::setNextTask(const TaskInfo &task)
{
    taskLock.lock();
    nextTask = task;
    waiter.wakeAll();
    taskLock.unlock();
}

void Worker::poisonPill()
{
    poisoned = true;
    waiter.wakeAll();
}

void Worker::run()
{
    while (!poisoned) {
        taskLock.lock();
        if (!nextTask.has_value()) {
            waiter.wait(&taskLock);
            if (poisoned) {
                taskLock.unlock();
                break;
            }
        }
        TaskInfo task = nextTask.value();
        nextTask.reset();
        taskLock.unlock();
        task.task();
        TasksDispatcher::instance()->d_ptr->taskFinished(id, task);
        QThread::yieldCurrentThread();
    }
}

#include "tasks.moc"
