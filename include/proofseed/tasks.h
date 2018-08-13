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
#ifndef PROOF_TASKS_H
#define PROOF_TASKS_H

#include "proofseed/future.h"
#include "proofseed/proofalgorithms.h"
#include "proofseed/proofseed_global.h"

#include "3rdparty/optional/optional.hpp"

#include <QEventLoop>
#include <QVector>

#include <cmath>
#include <type_traits>

namespace Proof {
namespace tasks {

enum class RestrictionType
{
    Custom,
    Intensive,
    ThreadBound
};

class TasksDispatcherPrivate;
class Worker;
class PROOF_SEED_EXPORT TasksDispatcher
{
    Q_DECLARE_PRIVATE(TasksDispatcher)
public:
    static TasksDispatcher *instance();
    TasksDispatcher(const TasksDispatcher &) = delete;
    TasksDispatcher(const TasksDispatcher &&) = delete;
    TasksDispatcher operator=(const TasksDispatcher &) = delete;
    TasksDispatcher operator=(const TasksDispatcher &&) = delete;

    qint32 capacity() const;
    qint32 restrictorCapacity(RestrictionType restrictionType, const QString &restrictor = QString()) const;
    void setCapacity(qint32 capacity);
    void addCustomRestrictor(const QString &restrictor, qint32 capacity);

    template <typename Task, typename Result = typename std::result_of_t<Task()>,
              typename = typename std::enable_if_t<!std::is_same<Result, void>::value
                                                   && !IsSpecialization<Result, QSharedPointer, Future>::value>>
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
              typename InnerResult = typename Result::Type::Value,
              typename = typename std::enable_if_t<IsSpecialization<Result, QSharedPointer, Future>::value>>
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
                                               && !IsSpecialization<Output, QSharedPointer, Future>::value>>
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
    return Future<>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(Input)>,
          typename = typename std::enable_if_t<IsSpecialization<Output, QSharedPointer, Future>::value>,
          typename OutputValue = typename Output::Type::Value>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(*(data.cbegin())), FutureSP<Container<OutputValue>>())
{
    if (!data.size())
        return Future<Container<OutputValue>>::successful();
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType, restrictor](const Input &x) -> Output {
                                   return run([x, task]() -> Output { return task(x); }, restrictionType, restrictor);
                               });
    return Future<>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(Input)>,
          typename = typename std::enable_if_t<std::is_same<Output, void>::value>>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(*(data.cbegin())), FutureSP<bool>())
{
    if (!data.size())
        return Future<>::successful(true);
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType,
                                restrictor](const Input &x) -> FutureSP<bool> {
                                   return run([x, task]() { task(x); }, restrictionType, restrictor);
                               });
    return Future<>::sequence(seq)->andThenValue(true);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(long long, Input)>,
          typename = typename std::enable_if_t<!std::is_same<Output, void>::value
                                               && !IsSpecialization<Output, QSharedPointer, Future>::value>>
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
    return Future<>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(long long, Input)>,
          typename = typename std::enable_if_t<IsSpecialization<Output, QSharedPointer, Future>::value>,
          typename OutputValue = typename Output::Type::Value>
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
    return Future<>::sequence(seq);
}

template <template <typename...> class Container, typename Input, typename Task,
          typename Output = typename std::result_of_t<Task(long long, Input)>,
          typename = typename std::enable_if_t<std::is_same<Output, void>::value>>
auto run(const Container<Input> &data, Task &&task, RestrictionType restrictionType = RestrictionType::Custom,
         const QString &restrictor = QString()) -> decltype(task(0ll, *(data.cbegin())), FutureSP<bool>())
{
    if (!data.size())
        return Future<>::successful(true);
    auto seq = algorithms::map(data,
                               [task = std::forward<Task>(task), restrictionType,
                                restrictor](long long index, const Input &x) -> FutureSP<bool> {
                                   return run([index, x, task]() { task(index, x); }, restrictionType, restrictor);
                               });
    return Future<>::sequence(seq)->andThenValue(true);
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
