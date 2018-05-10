#include "proofcore/tasks.h"
#include "proofcore/spinlock.h"

#include <QThread>
#include <QString>
#include <QMutex>
#include <QWaitCondition>
#include <QCoreApplication>

#include <vector>
#include <list>
#include <set>
#include <map>

constexpr qint32 HTTP_CAPACITY = 6;
static const qint32 INTENSIVE_CAPACITY = qMax(QThread::idealThreadCount(), 1);
constexpr qint32 CUSTOM_CAPACITY = 16;
constexpr qint32 DEFAULT_TOTAL_CAPACITY = 64;

namespace Proof {
namespace tasks {

struct TaskInfo
{
    TaskInfo(std::function<void()> &&_task, RestrictionType _type, const QString &_restrictor)
        : task(std::move(_task)), restrictionType(_type), restrictor(_restrictor)
    {
    }
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
    TasksDispatcher *q_ptr;
public:
    void taskFinished(qint32 workerId, const TaskInfo &taskInfo);
    std::vector<Worker *> workers;

private:
    void schedule(qint32 forcedWorkerId = -1); //should be always called under mainLock

    std::set<QThread *> poolThreads;
    std::set<qint32> waitingWorkers;
    std::list<TaskInfo> tasks;
    std::map<RestrictionType, std::map<QString, qint32>> restrictorsUsage;
    SpinLock mainLock;
    std::map<QString, qint32> customRestrictorsCapacity;
    quint32 capacity = DEFAULT_TOTAL_CAPACITY;

    static thread_local bool currentEventLoopStarted;
    static thread_local QSharedPointer<QEventLoop> signalWaitersEventLoop;
};
thread_local bool TasksDispatcherPrivate::currentEventLoopStarted = false;
thread_local QSharedPointer<QEventLoop> TasksDispatcherPrivate::signalWaitersEventLoop;

class Worker : public QThread
{
    Q_OBJECT
public:
    Worker(qint32 id);
    void setNextTask(const TaskInfo &task);
    void poisonPill();
protected:
    void run() override;
private:
    qint32 id = 0;
    std::atomic_bool poisoned {false};
    std::atomic_bool asleep {false};
    std::experimental::optional<TaskInfo> nextTask;
    QWaitCondition waiter;
    QMutex taskLock;
};

}
}

using namespace Proof::tasks;

TasksDispatcher::TasksDispatcher()
    : d_ptr(new TasksDispatcherPrivate)
{
    d_ptr->q_ptr = this;
}

TasksDispatcher::~TasksDispatcher()
{
    d_ptr->mainLock.lock();
    for (auto worker : d_ptr->workers)
        worker->poisonPill();
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
    if (restrictionType == RestrictionType::Intensive)
        return INTENSIVE_CAPACITY;
    else if (restrictor.isEmpty())
        return capacity();
    else if (restrictionType == RestrictionType::Http)
        return HTTP_CAPACITY;
    d_ptr->mainLock.lock();
    auto resultIt = d_ptr->customRestrictorsCapacity.find(restrictor);
    qint32 result = d_ptr->customRestrictorsCapacity.cend() == resultIt ? CUSTOM_CAPACITY : resultIt->second;
    d_ptr->mainLock.unlock();
    return result;
}

void TasksDispatcher::setCapacity(quint32 capacity)
{
    d_ptr->mainLock.lock();
    if (d_ptr->workers.size() <= capacity)
        d_ptr->capacity = capacity;
    d_ptr->mainLock.unlock();
}

void TasksDispatcher::addCustomRestrictor(const QString &restrictor, qint32 capacity)
{
    d_ptr->mainLock.lock();
    d_ptr->customRestrictorsCapacity[restrictor] = capacity;
    d_ptr->mainLock.unlock();
}

void TasksDispatcher::fireSignalWaiters()
{
    Q_ASSERT_X(d_ptr->poolThreads.count(QThread::currentThread()), "fireSignalWaiters", "signal waiters can't be used outside of tasks thread pool");
    qApp->processEvents();
    if (!d_ptr->signalWaitersEventLoop)
        return;
    d_ptr->currentEventLoopStarted = true;
    d_ptr->signalWaitersEventLoop->exec();
    clearEventLoop();
    qCDebug(proofCoreTasksExtraLog) << "Thread" << QThread::currentThread() << ": signal waiters fired";
}

void TasksDispatcher::insertTaskInfo(std::function<void ()> &&wrappedTask, RestrictionType restrictionType, const QString &restrictor)
{
    d_ptr->mainLock.lock();
    //We mimic all intensive tasks as under single restrictor
    d_ptr->tasks.push_back(TaskInfo(std::move(wrappedTask), restrictionType,
                                    restrictionType == RestrictionType::Intensive ? QLatin1String("_") : restrictor));
    d_ptr->schedule();
    d_ptr->mainLock.unlock();
}

void TasksDispatcher::addSignalWaiterPrivate(std::function<void (const QSharedPointer<QEventLoop> &)> &&connector)
{
    Q_ASSERT_X(d_ptr->poolThreads.count(QThread::currentThread()), "addSignalWaiterPrivate", "signal waiters can't be used outside of tasks thread pool");
    if (!d_ptr->signalWaitersEventLoop) {
        d_ptr->currentEventLoopStarted = false;
        d_ptr->signalWaitersEventLoop.reset(new QEventLoop);
    }
    connector(d_ptr->signalWaitersEventLoop);
}

bool TasksDispatcher::eventLoopStarted() const
{
    return d_ptr->currentEventLoopStarted;
}

void TasksDispatcher::clearEventLoop()
{
    d_ptr->signalWaitersEventLoop.clear();
    d_ptr->currentEventLoopStarted = false;
}

void TasksDispatcherPrivate::taskFinished(qint32 workerId, const TaskInfo &taskInfo)
{
    mainLock.lock();
    waitingWorkers.insert(workerId);
    if (restrictorsUsage[taskInfo.restrictionType][taskInfo.restrictor] <= 1)
        restrictorsUsage[taskInfo.restrictionType].erase(taskInfo.restrictor);
    else
        --restrictorsUsage[taskInfo.restrictionType][taskInfo.restrictor];
    schedule(workerId);
    mainLock.unlock();
}

void TasksDispatcherPrivate::schedule(qint32 forcedWorkerId)
{
    if (waitingWorkers.empty()) {
        if (workers.size() >= capacity)
            return;
        waitingWorkers.insert(workers.size());
        auto worker = new Worker(workers.size());
        workers.push_back(worker);
        poolThreads.insert(worker);
        worker->start();
    }

    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        if (!it->restrictor.isEmpty()) {
            qint32 capacityLeft = capacity;
            switch (it->restrictionType) {
            case RestrictionType::Http:
                capacityLeft = HTTP_CAPACITY;
                break;
            case RestrictionType::Intensive:
                capacityLeft = INTENSIVE_CAPACITY;
                break;
            case RestrictionType::Custom: {
                auto resultIt = customRestrictorsCapacity.find(it->restrictor);
                capacityLeft = customRestrictorsCapacity.cend() == resultIt ? CUSTOM_CAPACITY : resultIt->second;
                break;
            }
            }
            auto resultIt = restrictorsUsage[it->restrictionType].find(it->restrictor);
            capacityLeft -= restrictorsUsage[it->restrictionType].cend() == resultIt ? 0 : resultIt->second;
            if (capacityLeft <= 0)
                continue;
            ++restrictorsUsage[it->restrictionType][it->restrictor];
        }
        qint32 workerId = (forcedWorkerId >= 0 && waitingWorkers.count(forcedWorkerId)) ? forcedWorkerId : *waitingWorkers.cbegin();
        auto task = *it;
        tasks.erase(it);
        waitingWorkers.erase(workerId);
        workers[workerId]->setNextTask(task);
        break;
    }
}

Worker::Worker(qint32 id)
    : QThread(nullptr), id(id)
{
}

void Worker::setNextTask(const TaskInfo &task)
{
    taskLock.lock();
    nextTask = task;
    if (asleep)
        waiter.wakeAll();
    taskLock.unlock();
}

void Worker::poisonPill()
{
    poisoned = true;
}

void Worker::run()
{
    while (!poisoned) {
        taskLock.lock();
        if (!nextTask.has_value()) {
            asleep = true;
            waiter.wait(&taskLock);
        }
        TaskInfo task = nextTask.value();
        nextTask.reset();
        taskLock.unlock();
        task.task();
        TasksDispatcher::instance()->d_ptr->taskFinished(id, task);
    }
    delete this;
}

#include "tasks.moc"
