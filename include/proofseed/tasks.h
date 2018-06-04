#ifndef PROOF_TASKS_H
#define PROOF_TASKS_H

#include "proofcore/3rdparty/optional.hpp"
#include "proofcore/future.h"
#include "proofcore/helpers/zipper.h"
#include "proofcore/proofalgorithms.h"
#include "proofcore/proofcore_global.h"

#include <QEventLoop>

#include <cmath>
#include <type_traits>

namespace Proof {
namespace tasks {

enum class RestrictionType
{
    Custom,
    Intensive,
    Http, //Deprecated, will be removed soon
    ThreadBound
};

class TasksDispatcherPrivate;
class Worker;
class PROOF_CORE_EXPORT TasksDispatcher
{
    Q_DECLARE_PRIVATE(TasksDispatcher)
public:
    static TasksDispatcher *instance();
    TasksDispatcher(const TasksDispatcher &) = delete;
    TasksDispatcher(const TasksDispatcher &&) = delete;
    TasksDispatcher operator=(const TasksDispatcher &) = delete;
    TasksDispatcher operator=(const TasksDispatcher &&) = delete;

    qint32 capacity() const;
    qint32 restrictorCapacity(RestrictionType restrictionType, const QString &restrictor = QLatin1String()) const;
    void setCapacity(qint32 capacity);
    void addCustomRestrictor(const QString &restrictor, qint32 capacity);

    template <typename Task, typename Result = typename std::result_of_t<Task()>,
              typename = typename std::enable_if_t<!std::is_same<Result, void>::value
                                                   && !detail::IsSpecialization2<Result, QSharedPointer, Future>::value>>
    CancelableFuture<Result> run(Task &&task, RestrictionType restrictionType, const QString &restrictor)
    {
        PromiseSP<Result> promise = PromiseSP<Result>::create();
        std::function<void()> f = [promise, task = std::forward<Task>(task)]() {
            if (promise->filled())
                return;
            Proof::futures::detail::resetLastFailure();
            promise->success(task());
        };
        insertTaskInfo(std::move(f), restrictionType, restrictor);
        return CancelableFuture<Result>(promise);
    }

    template <typename Task, typename Result = typename std::result_of_t<Task()>,
              typename InnerResult = typename std::decay<decltype(*Result().data())>::type::Value,
              typename = typename std::enable_if_t<detail::IsSpecialization2<Result, QSharedPointer, Future>::value>>
    CancelableFuture<InnerResult> run(Task &&task, RestrictionType restrictionType, const QString &restrictor)
    {
        PromiseSP<InnerResult> promise = PromiseSP<InnerResult>::create();
        std::function<void()> f = [promise, task = std::forward<Task>(task)]() {
            if (promise->filled())
                return;
            Proof::futures::detail::resetLastFailure();
            task()
                ->onSuccess([promise](const InnerResult &result) { promise->success(result); })
                ->onFailure([promise](const Failure &failure) { promise->failure(failure); });
        };
        insertTaskInfo(std::move(f), restrictionType, restrictor);
        return CancelableFuture<InnerResult>(promise);
    }

    template <typename Task, typename Result = typename std::result_of_t<Task()>,
              typename = typename std::enable_if_t<std::is_same<Result, void>::value>>
    CancelableFuture<bool> run(Task &&task, RestrictionType restrictionType, const QString &restrictor)
    {
        PromiseSP<bool> promise = PromiseSP<bool>::create();
        std::function<void()> f = [promise, task = std::forward<Task>(task)]() {
            if (promise->filled())
                return;
            Proof::futures::detail::resetLastFailure();
            task();
            promise->success(true);
        };
        insertTaskInfo(std::move(f), restrictionType, restrictor);
        return CancelableFuture<bool>(promise);
    }

    template <class SignalSender, class SignalType, class... Args>
    void addSignalWaiter(SignalSender *sender, SignalType signal, std::function<bool(Args...)> callback)
    {
        std::function<void(const QSharedPointer<QEventLoop> &)> connector =
            [sender, signal, callback, this](const QSharedPointer<QEventLoop> &eventLoop) {
                auto connection = QSharedPointer<QMetaObject::Connection>::create();
                auto eventLoopWeak = eventLoop.toWeakRef();
                std::function<void(Args...)> slot = [callback, eventLoopWeak, connection, this](Args... args) {
                    QSharedPointer<QEventLoop> eventLoop = eventLoopWeak.toStrongRef();
                    if (!eventLoop)
                        return;
                    if (!callback(args...))
                        return;
                    QObject::disconnect(*connection);
                    if (!eventLoopStarted())
                        clearEventLoop();
                    else
                        eventLoop->quit();
                };
                *connection = QObject::connect(sender, signal, eventLoop.data(), slot, Qt::QueuedConnection);
            };
        qCDebug(proofCoreTasksExtraLog) << "Thread" << QThread::currentThread()
                                        << ": signal waiter added, sender:" << sender << "signal:" << &signal;
        addSignalWaiterPrivate(std::move(connector));
    }

    void fireSignalWaiters();

private:
    friend class Worker;
    TasksDispatcher();
    ~TasksDispatcher();
    void insertTaskInfo(std::function<void()> &&wrappedTask, RestrictionType restrictionType, const QString &restrictor);
    void addSignalWaiterPrivate(std::function<void(const QSharedPointer<QEventLoop> &)> &&connector);
    bool eventLoopStarted() const;
    void clearEventLoop();

    QScopedPointer<TasksDispatcherPrivate> d_ptr;
};

template <typename Task>
auto run(Task &&task, RestrictionType restrictionType = RestrictionType::Custom, const QString &restrictor = QString())
    -> decltype(TasksDispatcher::instance()->run(task, restrictionType, restrictor))
{
    return TasksDispatcher::instance()->run(std::forward<Task>(task), restrictionType, restrictor);
}

template <typename Task>
auto run(RestrictionType restrictionType, const QString &restrictor, Task &&task)
    -> decltype(TasksDispatcher::instance()->run(task, restrictionType, restrictor))
{
    return TasksDispatcher::instance()->run(std::forward<Task>(task), restrictionType, restrictor);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(Input)>,
          typename = typename std::enable_if_t<!std::is_same<Output, void>::value
                                               && !detail::IsSpecialization2<Output, QSharedPointer, Future>::value>>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(*(data.cbegin())), FutureSP<Container<Output>>())
{
    if (!data.size())
        return Future<Container<Output>>::successful();
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType,
                                restrictor](const Input &x) -> FutureSP<Output> {
                                   return run([x, task]() -> Output { return task(x); }, restrictionType, restrictor);
                               });
    return Future<Output>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(Input)>,
          typename = typename std::enable_if_t<detail::IsSpecialization2<Output, QSharedPointer, Future>::value>,
          typename OutputValue = typename std::decay<decltype(*Output().data())>::type::Value>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(*(data.cbegin())), FutureSP<Container<OutputValue>>())
{
    if (!data.size())
        return Future<Container<OutputValue>>::successful();
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType, restrictor](const Input &x) -> Output {
                                   return run([x, task]() -> Output { return task(x); }, restrictionType, restrictor);
                               });
    return Future<OutputValue>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(Input)>,
          typename = typename std::enable_if_t<std::is_same<Output, void>::value>>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(*(data.cbegin())), FutureSP<bool>())
{
    if (!data.size())
        return Future<bool>::successful(true);
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType,
                                restrictor](const Input &x) -> FutureSP<bool> {
                                   return run([x, task]() { task(x); }, restrictionType, restrictor);
                               });
    return Future<bool>::sequence(seq)->map([](const auto &) { return true; });
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(long long, Input)>,
          typename = typename std::enable_if_t<!std::is_same<Output, void>::value
                                               && !detail::IsSpecialization2<Output, QSharedPointer, Future>::value>>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(0ll, *(data.cbegin())), FutureSP<Container<Output>>())
{
    if (!data.size())
        return Future<Container<Output>>::successful();
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType,
                                restrictor](long long index, const Input &x) -> FutureSP<Output> {
                                   return run([index, x, task]() -> Output { return task(index, x); }, restrictionType,
                                              restrictor);
                               });
    return Future<Output>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(long long, Input)>,
          typename = typename std::enable_if_t<detail::IsSpecialization2<Output, QSharedPointer, Future>::value>,
          typename OutputValue = typename std::decay<decltype(*Output().data())>::type::Value>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString())
    -> decltype(task(0ll, *(data.cbegin())), FutureSP<Container<OutputValue>>())
{
    if (!data.size())
        return Future<Container<OutputValue>>::successful();
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType,
                                restrictor](long long index, const Input &x) -> Output {
                                   return run([index, x, task]() -> Output { return task(index, x); }, restrictionType,
                                              restrictor);
                               });
    return Future<OutputValue>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(long long, Input)>,
          typename = typename std::enable_if_t<std::is_same<Output, void>::value>>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(0ll, *(data.cbegin())), FutureSP<bool>())
{
    if (!data.size())
        return Future<bool>::successful(true);
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType,
                                restrictor](long long index, const Input &x) -> FutureSP<bool> {
                                   return run([index, x, task]() { task(index, x); }, restrictionType, restrictor);
                               });
    return Future<bool>::sequence(seq)->map([](const auto &) { return true; });
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(Input)>>
auto clusteredRun(const Container<Input> &data, Task &&task, qint64 minClusterSize = 1,
                  RestrictionType restrictionType = RestrictionType::Custom, const QString &restrictor = QString())
    -> decltype((Container<Output>()).resize(data.count()), FutureSP<Container<Output>>())
{
    if (!data.size())
        return Future<Container<Output>>::successful();
    if (minClusterSize <= 0)
        minClusterSize = 1;

    return run([data, task = std::forward<Task>(task), minClusterSize, restrictionType, restrictor]() -> Container<Output> {
        qint64 amount = data.count();
        qint32 capacity = TasksDispatcher::instance()->restrictorCapacity(restrictionType, restrictor);
        if (capacity == TasksDispatcher::instance()->capacity())
            capacity = qMax(capacity / 2, TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Intensive));
        capacity = qMin(static_cast<qint32>(ceil(amount / static_cast<double>(minClusterSize))), capacity);
        qint32 clusterSize = static_cast<qint32>(amount / capacity);
        Container<Output> result;
        result.resize(amount);
        QVector<FutureSP<bool>> futures;
        futures.reserve(capacity);
        for (qint32 job = 0; job < capacity; ++job) {
            qint32 right = job == (capacity - 1) ? amount : ((job + 1) * clusterSize + 1);
            futures << run([&data, &task, &result, left = job * clusterSize, right]() {
                for (qint32 i = left; i < right; ++i)
                    result[i] = task(data[i]);
            });
        }
        for (const auto &future : qAsConst(futures))
            future->wait();
        for (const auto &future : qAsConst(futures)) {
            if (future->failed())
                return WithFailure(future->failureReason());
        }
        return result;
    });
}

template <typename SignalSender, typename SignalType, typename... Args>
void addSignalWaiter(SignalSender *sender, SignalType signal, std::function<bool(Args...)> callback)
{
    TasksDispatcher::instance()->addSignalWaiter(sender, signal, callback);
}

inline void fireSignalWaiters()
{
    TasksDispatcher::instance()->fireSignalWaiters();
}
} // namespace tasks
} // namespace Proof

#endif // PROOF_TASKS_H
