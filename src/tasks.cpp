#include "proofcore/tasks.h"
#include "proofcore/spinlock.h"

#include <QThread>
#include <QString>
#include <QLinkedList>
#include <QMap>
#include <QVector>
#include <QSet>
#include <QMutex>
#include <QWaitCondition>
#include <QCoreApplication>

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
    QVector<Worker *> workers;

private:
    void schedule(qint32 forcedWorkerId = -1); //should be always called under mainLock

    QSet<QThread *> poolThreads;
    QSet<qint32> waitingWorkers;
    QLinkedList<TaskInfo> tasks;
    QMap<RestrictionType, QMap<QString, qint32>> restrictorsUsage;
    SpinLock mainLock;
    QMap<QString, qint32> customRestrictorsCapacity;
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
    Worker(qint32 id);
    void setNextTask(const TaskInfo &task);
    void poisonPill();
protected:
    void run() override;
private:
    qint32 id = 0;
    std::experimental::optional<TaskInfo> nextTask;
    QMutex taskLock;
    std::atomic_bool poisoned {false};
    std::atomic_bool asleep {false};
    QWaitCondition waiter;
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
    qint32 result = d_ptr->customRestrictorsCapacity.value(restrictor, CUSTOM_CAPACITY);
    d_ptr->mainLock.unlock();
    return result;
}

void TasksDispatcher::setCapacity(qint32 capacity)
{
    d_ptr->mainLock.lock();
    if (d_ptr->workers.count() <= capacity)
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
    Q_ASSERT_X(d_ptr->poolThreads.contains(QThread::currentThread()), "fireSignalWaiters", "signal waiters can't be used outside of tasks thread pool");
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
    d_ptr->tasks << TaskInfo(std::move(wrappedTask), restrictionType,
                             restrictionType == RestrictionType::Intensive ? QLatin1String("_") : restrictor);
    d_ptr->schedule();
    d_ptr->mainLock.unlock();
}

void TasksDispatcher::addSignalWaiterPrivate(std::function<void (const QSharedPointer<QEventLoop> &)> &&connector)
{
    Q_ASSERT_X(d_ptr->poolThreads.contains(QThread::currentThread()), "addSignalWaiterPrivate", "signal waiters can't be used outside of tasks thread pool");
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
        restrictorsUsage[taskInfo.restrictionType].remove(taskInfo.restrictor);
    else
        --restrictorsUsage[taskInfo.restrictionType][taskInfo.restrictor];
    schedule(workerId);
    mainLock.unlock();
}

void TasksDispatcherPrivate::schedule(qint32 forcedWorkerId)
{
    if (waitingWorkers.isEmpty()) {
        if (workers.count() >= capacity)
            return;
        waitingWorkers.insert(workers.count());
        auto worker = new Worker(workers.count());
        workers << worker;
        poolThreads << worker;
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
            case RestrictionType::Custom:
                capacityLeft = customRestrictorsCapacity.value(it->restrictor, CUSTOM_CAPACITY);
                break;
            }
            capacityLeft -= restrictorsUsage[it->restrictionType].value(it->restrictor, 0);
            if (capacityLeft <= 0)
                continue;
            ++restrictorsUsage[it->restrictionType][it->restrictor];
        }
        qint32 workerId = (forcedWorkerId >= 0 && waitingWorkers.contains(forcedWorkerId)) ? forcedWorkerId : *waitingWorkers.cbegin();
        workers[workerId]->setNextTask(*it);
        tasks.erase(it);
        waitingWorkers.remove(workerId);
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
