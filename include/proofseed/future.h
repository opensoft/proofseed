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
#ifndef PROOF_FUTURE_H
#define PROOF_FUTURE_H

#include "proofseed/helpers/tuplemaker.h"
#include "proofseed/proofalgorithms.h"
#include "proofseed/proofseed_global.h"
#include "proofseed/spinlock.h"

#include "3rdparty/optional/optional.hpp"

#include <QCoreApplication>
#include <QMutex>
#include <QSharedPointer>
#include <QString>
#include <QThread>
#include <QTime>
#include <QVariant>
#include <QWaitCondition>
#include <QWeakPointer>

#include <list>

namespace Proof {
//TODO: add Promise<void>, Future<void> if ever will be needed
template <typename... T>
class Future;
template <typename T>
using FutureSP = QSharedPointer<Future<T>>;
template <typename T>
using FutureWP = QWeakPointer<Future<T>>;

struct Failure
{
    enum Hints
    {
        NoHint = 0x0,
        UserFriendlyHint = 0x1,
        CriticalHint = 0x2,
        DataIsHttpCodeHint = 0x4,
        FromExceptionHint = 0x8
    };
    Failure(const QString &message, long moduleCode, long errorCode, unsigned long hints = Failure::NoHint,
            const QVariant &data = QVariant()) noexcept
        : exists(true), moduleCode(moduleCode), errorCode(errorCode), hints(hints), message(message), data(data)
    {}
    explicit Failure(const QVariant &data) noexcept : exists(true), data(data) {}
    Failure() noexcept {}
    Failure(Failure &&) noexcept = default;
    Failure(const Failure &) = default;
    ~Failure() = default;
    Failure &operator=(Failure &&) noexcept = default;
    Failure &operator=(const Failure &) = default;
    operator QString() noexcept { return message; } // NOLINT(google-explicit-constructor)
    Failure withMessage(const QString &msg) const noexcept { return Failure(msg, moduleCode, errorCode, hints, data); }
    Failure withCode(long module, long error) const noexcept { return Failure(message, module, error, hints, data); }
    Failure withData(const QVariant &d) const noexcept { return Failure(message, moduleCode, errorCode, hints, d); }
    static inline Failure fromException(const std::exception &e)
    {
        return Failure(QStringLiteral("Exception caught: %1").arg(e.what()), 0, 0, Failure::FromExceptionHint);
    }
    static inline Failure fromException()
    {
        return Failure(QStringLiteral("Exception caught"), 0, 0, Failure::FromExceptionHint);
    }
    bool exists = false;
    long moduleCode = 0;
    long errorCode = 0;
    unsigned long hints = NoHint;
    QString message;
    QVariant data;
};

namespace futures {
namespace detail {
template <typename...>
struct Zipper;

template <typename Left, typename Right>
struct Zipper<std::enable_if_t<IsSpecialization<Left, std::tuple>::value, std::true_type>, Left, Right>
{
    using type = decltype(std::tuple_cat(Left(), typename Proof::detail::TupleMaker<typename Right::Type::Value>::type()));
};

template <typename Left, typename Right>
struct Zipper<std::enable_if_t<!IsSpecialization<Left, std::tuple>::value, std::true_type>, Left, Right>
{
    using type = decltype(std::tuple_cat(typename Proof::detail::TupleMaker<typename Left::Type::Value>::type(),
                                         typename Proof::detail::TupleMaker<typename Right::Type::Value>::type()));
};

template <typename Left, typename Middle, typename Right, typename... Tail>
struct Zipper<std::true_type, Left, Middle, Right, Tail...>
{
    using type = typename Zipper<std::true_type, typename Zipper<std::true_type, Left, Middle>::type, Right, Tail...>::type;
};

bool PROOF_SEED_EXPORT hasLastFailure() noexcept;
Failure PROOF_SEED_EXPORT lastFailure() noexcept;
void PROOF_SEED_EXPORT resetLastFailure() noexcept;
void PROOF_SEED_EXPORT setLastFailure(Failure &&failure) noexcept;
void PROOF_SEED_EXPORT setLastFailure(const Failure &failure) noexcept;
} // namespace detail
} // namespace futures

//This helper struct is designed to be used only in next cases:
//1. return clause of Future::map()/Future::reduce()
//2. Promise::success argument
//3. tasks::run() family
//All other usages will lead to undefined behavior
//Storing and/or casting to T/FutureSP<T> explicitly will lead to undefined behavior
struct WithFailure
{
    explicit WithFailure(const Failure &f) noexcept { m_failure = f; }
    explicit WithFailure(Failure &&f) noexcept { m_failure = std::move(f); }
    template <typename... Args>
    explicit WithFailure(Args &&... args) noexcept
    {
        m_failure = Failure(std::forward<Args>(args)...);
    }
    template <typename T>
    operator T() noexcept // NOLINT(google-explicit-constructor)
    {
        futures::detail::setLastFailure(std::move(m_failure));
        return T();
    }
    template <typename T>
    operator FutureSP<T>() noexcept // NOLINT(google-explicit-constructor)
    {
        FutureSP<T> result = Future<T>::create();
        result->fillFailure(std::move(m_failure));
        return result;
    }

private:
    Failure m_failure;
};

template <typename T>
class Promise
{
public:
    Promise() noexcept = default;
    Promise(const Promise<T> &) = delete;
    Promise(Promise<T> &&) = delete;
    Promise &operator=(const Promise<T> &) = delete;
    Promise &operator=(Promise<T> &&) = delete;
    ~Promise() = default;

    FutureSP<T> future() const { return m_future; }

    bool filled() noexcept { return m_future->completed(); }
    void failure(const Failure &reason) noexcept { m_future->fillFailure(reason); }
    void failure(Failure &&reason) noexcept { m_future->fillFailure(reason); }
    void success(const T &result) noexcept { m_future->fillSuccess(result); }

private:
    FutureSP<T> m_future = Future<T>::create();
};
template <typename T>
using PromiseSP = QSharedPointer<Promise<T>>;
template <typename T>
using PromiseWP = QWeakPointer<Promise<T>>;

template <typename T>
class CancelableFuture
{
public:
    using Type = Future<T>;
    using element_type = Future<T>;
    using value_type = Future<T>;
    explicit CancelableFuture() noexcept { m_promise = PromiseSP<T>::create(); }
    explicit CancelableFuture(const PromiseSP<T> &promise) noexcept { m_promise = promise; }
    CancelableFuture(CancelableFuture &&) noexcept = default;
    CancelableFuture(const CancelableFuture &) noexcept = default;
    CancelableFuture &operator=(CancelableFuture &&) noexcept = default;
    CancelableFuture &operator=(const CancelableFuture &) noexcept = default;
    ~CancelableFuture() = default;
    void cancel(const Failure &failure = Failure(QStringLiteral("Canceled"), 0, 0)) const noexcept
    {
        if (!m_promise->filled())
            m_promise->failure(failure);
    }
    Future<T> *operator->() const noexcept { return m_promise->future().data(); }
    operator FutureSP<T>() const noexcept { return m_promise->future(); } // NOLINT(google-explicit-constructor)
    FutureSP<T> future() const noexcept { return m_promise->future(); }

private:
    PromiseSP<T> m_promise;
};

template <typename T>
class Future<T>
{
    template <typename... U>
    friend class Future;
    friend class Promise<T>;
    friend struct WithFailure;

public:
    using Value = T;

    Future(const Future<T> &) = delete;
    Future(Future<T> &&) = delete;
    Future &operator=(const Future<T> &) = delete;
    Future &operator=(Future<T> &&) = delete;
    ~Future() = default;

    bool completed() const noexcept
    {
        int value = m_state.load(std::memory_order_acquire);
        return value == FailedFuture || value == SucceededFuture;
    }
    bool failed() const noexcept { return m_state.load(std::memory_order_acquire) == FailedFuture; }
    bool succeeded() const noexcept { return m_state.load(std::memory_order_acquire) == SucceededFuture; }

    bool wait(long long timeout = -1) const noexcept
    {
        auto self = m_weakSelf.toStrongRef();
        Q_ASSERT(self);
        if (completed())
            return true;
        bool waitForever = timeout < 1;
        bool maintainEvents = QThread::currentThread() == qApp->thread();
        if (maintainEvents || !waitForever) {
            QTime timer;
            timer.start();
            while (waitForever || (timer.elapsed() <= timeout)) {
                if (completed())
                    return true;
                if (maintainEvents)
                    QCoreApplication::processEvents();
                else
                    QThread::msleep(1);
            }
        } else {
            QMutex mutex;
            QWaitCondition waiter;
            mutex.lock();
            bool wasInSameThread = false;
            self->recover([](const Failure &) { return T(); })
                ->onSuccess([&waiter, &mutex, &wasInSameThread, waitingThread = QThread::currentThread()](const T &) {
                    if (QThread::currentThread() == waitingThread) {
                        wasInSameThread = true;
                        return;
                    }
                    mutex.lock();
                    mutex.unlock();
                    waiter.wakeAll();
                });
            if (!wasInSameThread)
                waiter.wait(&mutex);
            mutex.unlock();
        }
        return completed();
    }

    T result() const
    {
        if (!completed())
            wait();
        if (succeeded()) {
            Q_ASSERT(m_result.has_value());
            return m_result.value();
        }
        return T();
    }

    Failure failureReason() const
    {
        while (!completed())
            wait();
        if (failed()) {
            Q_ASSERT(m_failureReason.has_value());
            return m_failureReason.value();
        }
        return Failure();
    }

    template <typename Func>
    auto onSuccess(Func &&f) noexcept -> decltype(f(T()), FutureSP<T>())
    {
        bool callIt = false;
        m_mainLock.lock();
        if (completed())
            callIt = true;
        else
            m_successCallbacks.emplace_back(f);
        m_mainLock.unlock();

        if (callIt && succeeded()) {
            try {
                f(m_result.value());
            } catch (...) {
            }
        }
        return m_weakSelf.toStrongRef();
    }

    template <typename Func>
    auto onFailure(Func &&f) noexcept -> decltype(f(Failure()), FutureSP<T>())
    {
        bool callIt = false;
        m_mainLock.lock();
        if (completed())
            callIt = true;
        else
            m_failureCallbacks.emplace_back(f);
        m_mainLock.unlock();

        if (callIt && failed()) {
            try {
                f(failureReason());
            } catch (...) {
            }
        }
        return m_weakSelf.toStrongRef();
    }

    template <typename Func>
    auto forEach(Func &&f) noexcept -> decltype(f(T()), FutureSP<T>())
    {
        return onSuccess(std::forward<Func>(f));
    }

    template <typename Func>
    auto filter(Func &&f, const Failure &rejected = Failure(QStringLiteral("Result wasn't good enough"), 0, 0)) noexcept
        -> decltype(!!f(T()), FutureSP<T>())
    {
        FutureSP<T> result = Future<T>::create();
        onSuccess([result, f = std::forward<Func>(f), rejected](const T &v) noexcept {
            try {
                if (f(v))
                    result->fillSuccess(v);
                else
                    result->fillFailure(rejected);
            } catch (const std::exception &e) {
                result->fillFailure(Failure::fromException(e));
            } catch (...) {
                result->fillFailure(Failure::fromException());
            }
        });
        onFailure([result](const Failure &failure) noexcept { result->fillFailure(failure); });
        return result;
    }

    template <typename Func>
    auto map(Func &&f) noexcept -> decltype(FutureSP<decltype(f(T()))>())
    {
        using U = decltype(f(T()));
        FutureSP<U> result = Future<U>::create();
        onSuccess([result, f = std::forward<Func>(f)](const T &v) noexcept {
            try {
                result->fillSuccess(f(v));
            } catch (const std::exception &e) {
                result->fillFailure(Failure::fromException(e));
            } catch (...) {
                result->fillFailure(Failure::fromException());
            }
        });
        onFailure([result](const Failure &failure) noexcept { result->fillFailure(failure); });
        return result;
    }

    template <typename Func>
    auto flatMap(Func &&f) noexcept -> decltype(FutureSP<decltype(f(T())->result())>())
    {
        using U = decltype(f(T())->result());
        FutureSP<U> result = Future<U>::create();
        onSuccess([result, f = std::forward<Func>(f)](const T &v) noexcept {
            try {
                FutureSP<U> inner = f(v);
                inner->onSuccess([result](const U &v) noexcept { result->fillSuccess(v); });
                inner->onFailure([result](const Failure &failure) noexcept { result->fillFailure(failure); });
            } catch (const std::exception &e) {
                result->fillFailure(Failure::fromException(e));
            } catch (...) {
                result->fillFailure(Failure::fromException());
            }
        });
        onFailure([result](const Failure &failure) noexcept { result->fillFailure(failure); });
        return result;
    }

    template <typename Func>
    auto andThen(Func &&f) noexcept -> decltype(FutureSP<decltype(f()->result())>())
    {
        return flatMap([f = std::forward<Func>(f)](const T &) { return f(); });
    }

    template <typename T2, typename Result = typename std::decay<T2>::type>
    FutureSP<Result> andThenValue(T2 &&value) noexcept
    {
        return map([value = std::forward<T2>(value)](const auto &) noexcept { return value; });
    }

    template <typename Func, typename Result>
    auto innerReduce(Func &&f, Result acc) noexcept -> decltype(algorithms::reduce(T(), f, acc), FutureSP<Result>())
    {
        return map([f = std::forward<Func>(f), acc = std::move(acc)](const T &v) {
            return algorithms::reduce(v, f, std::move(acc));
        });
    }

    template <typename Func, typename Result>
    auto innerReduceByMutation(Func &&f, Result acc) noexcept
        -> decltype(algorithms::reduceByMutation(T(), f, acc), FutureSP<Result>())
    {
        return map([f = std::forward<Func>(f), acc = std::move(acc)](const T &v) {
            return algorithms::reduceByMutation(v, f, std::move(acc));
        });
    }

    template <typename Func, typename Result>
    auto innerMap(Func &&f, Result dest) noexcept -> decltype(algorithms::map(T(), f, dest), FutureSP<Result>())
    {
        return map([f = std::forward<Func>(f), dest = std::move(dest)](const T &v) {
            return algorithms::map(v, f, std::move(dest));
        });
    }

    template <typename Func>
    auto innerMap(Func &&f) noexcept -> decltype(algorithms::map(T(), f), FutureSP<decltype(algorithms::map(T(), f))>())
    {
        return map([f = std::forward<Func>(f)](const T &v) { return algorithms::map(v, f); });
    }

    template <typename Func>
    auto innerFilter(Func &&f) noexcept -> decltype(algorithms::filter(T(), f), FutureSP<T>())
    {
        return map([f = std::forward<Func>(f)](const T &v) { return algorithms::filter(v, f); });
    }

    template <typename Result>
    auto innerFlatten(Result acc) noexcept -> decltype(algorithms::flatten(T(), acc), FutureSP<Result>())
    {
        return map([acc = std::move(acc)](const T &v) { return algorithms::flatten(v, std::move(acc)); });
    }

    template <typename Dummy = void, typename = std::enable_if_t<NestingLevel<T>::value >= 2, Dummy>>
    auto innerFlatten() noexcept
    {
        return map([](const T &v) { return algorithms::flatten(v); });
    }

    template <typename Func>
    auto recover(Func &&f) noexcept -> decltype(f(Failure()), FutureSP<T>())
    {
        FutureSP<T> result = Future<T>::create();
        onSuccess([result](const T &v) noexcept { result->fillSuccess(v); });
        onFailure([result, f = std::forward<Func>(f)](const Failure &failure) noexcept {
            try {
                result->fillSuccess(f(failure));
            } catch (const std::exception &e) {
                result->fillFailure(Failure::fromException(e));
            } catch (...) {
                result->fillFailure(Failure::fromException());
            }
        });
        return result;
    }

    template <typename Func>
    auto recoverWith(Func &&f) noexcept -> decltype(f(Failure())->result(), FutureSP<T>())
    {
        FutureSP<T> result = Future<T>::create();
        onSuccess([result](const T &v) noexcept { result->fillSuccess(v); });
        onFailure([result, f = std::forward<Func>(f)](const Failure &failure) noexcept {
            try {
                FutureSP<T> inner = f(failure);
                inner->onSuccess([result](const T &v) noexcept { result->fillSuccess(v); });
                inner->onFailure([result](const Failure &failure) noexcept { result->fillFailure(failure); });
            } catch (const std::exception &e) {
                result->fillFailure(Failure::fromException(e));
            } catch (...) {
                result->fillFailure(Failure::fromException());
            }
        });
        return result;
    }

    FutureSP<T> recoverValue(T &&value) noexcept
    {
        return recover([value = std::forward<T>(value)](const Failure &) noexcept { return value; });
    }

    template <typename Head, typename... Other,
              typename Result = typename futures::detail::Zipper<std::true_type, FutureSP<T>, Head, Other...>::type>
    FutureSP<Result> zip(Head head, Other... other) noexcept
    {
        return flatMap([head, other...](const T &v) noexcept->FutureSP<Result> {
            return head->zip(other...)->map([v](const auto &argsResult) noexcept->Result {
                return std::tuple_cat(detail::TupleMaker<T>::result(v), argsResult);
            });
        });
    }

    template <typename T2, typename Result = typename futures::detail::Zipper<std::true_type, FutureSP<T>, FutureSP<T2>>::type>
    FutureSP<Result> zipValue(const T2 &value) noexcept
    {
        return zip(Future<T2>::successful(value));
    }

    static auto successful() noexcept -> decltype(T(), FutureSP<T>())
    {
        FutureSP<T> result = create();
        result->fillSuccess(T());
        return result;
    }

    static FutureSP<T> successful(const T &value) noexcept
    {
        FutureSP<T> result = create();
        result->fillSuccess(value);
        return result;
    }

    static FutureSP<T> fail(const Failure &failure) noexcept
    {
        FutureSP<T> result = create();
        result->fillFailure(failure);
        return result;
    }

    //This method was made non-template to restrict usage only to linear qt containers because of container copying
    static FutureSP<QVector<T>> sequence(QVector<FutureSP<T>> container) noexcept
    {
        if (container.isEmpty())
            return Future<QVector<T>>::successful();
        PromiseSP<QVector<T>> promise = PromiseSP<QVector<T>>::create();
        QVector<T> result;
        result.reserve(container.count());
        iterateSequence(std::move(container), container.cbegin(), std::move(result), promise);
        return promise->future();
    }

private:
    // TODO: return noexcept after moving android builds to clang
    Future() = default;
    static FutureSP<T> create() noexcept
    {
        //in-place creation is a trade off for future creation restriction
        FutureSP<T> result(new Future<T>);
        result->m_weakSelf = result.toWeakRef();
        return result;
    }

    void fillSuccess(const T &result) noexcept
    {
        if (futures::detail::hasLastFailure()) {
            auto failure = futures::detail::lastFailure();
            futures::detail::resetLastFailure();
            fillFailure(failure);
            return;
        }

        m_mainLock.lock();
        if (completed()) {
            m_mainLock.unlock();
            return;
        }
        m_result = result;
        m_state.store(SucceededFuture, std::memory_order_release);

        std::list<std::function<void(const T &)>> callbacks;
        std::swap(callbacks, m_successCallbacks);
        m_failureCallbacks.clear();
        m_mainLock.unlock();
        for (const auto &f : callbacks) {
            try {
                f(result);
            } catch (...) {
            }
        }
    }

    void fillFailure(const Failure &reason) noexcept
    {
        m_mainLock.lock();
        if (completed()) {
            m_mainLock.unlock();
            return;
        }
        m_failureReason = reason;
        m_state.store(FailedFuture, std::memory_order_release);

        std::list<std::function<void(const Failure &)>> callbacks;
        std::swap(callbacks, m_failureCallbacks);
        m_successCallbacks.clear();
        m_mainLock.unlock();
        for (const auto &f : callbacks) {
            try {
                f(reason);
            } catch (...) {
            }
        }
    }

    auto zip() noexcept
    {
        return map([](const T &v) noexcept { return detail::TupleMaker<T>::result(v); });
    }

    template <typename It, template <typename...> class Container>
    static void iterateSequence(Container<FutureSP<T>> &&initial, It current, Container<T> &&result,
                                const PromiseSP<Container<T>> &promise) noexcept
    {
        while ((*current)->completed()) {
            if ((*current)->failed()) {
                promise->failure((*current)->failureReason());
                return;
            }
            result << (*current)->result();
            ++current;
            if (current == initial.cend()) {
                promise->success(std::move(result));
                return;
            }
        }
        auto currentFuture = *current;
        currentFuture->onSuccess(
            sequenceSuccessListenerGenerator(std::move(initial), current, std::move(result), promise));
        currentFuture->onFailure([promise](const Failure &reason) noexcept { promise->failure(reason); });
    }

    template <typename It, template <typename...> class Container>
    static std::function<void(const T &)>
    sequenceSuccessListenerGenerator(Container<FutureSP<T>> &&initial, It current, Container<T> &&result,
                                     const PromiseSP<Container<T>> &promise) noexcept
    {
        return [initial = std::move(initial), current, result = std::move(result), promise](const T &v) mutable noexcept
        {
            ++current;
            result << v;
            if (current == initial.cend())
                promise->success(std::move(result));
            else
                iterateSequence(std::move(initial), current, std::move(result), promise);
        };
    }

    enum State
    {
        NotCompletedFuture = 0,
        SucceededFuture = 1,
        FailedFuture = 2
    };

    std::atomic_int m_state{NotCompletedFuture};
    std::experimental::optional<T> m_result;
    std::experimental::optional<Failure> m_failureReason;

    std::list<std::function<void(const T &)>> m_successCallbacks;
    std::list<std::function<void(const Failure &)>> m_failureCallbacks;
    SpinLock m_mainLock;
    FutureWP<T> m_weakSelf;
};

template <>
class Future<>
{
public:
    Future() = delete;
    Future(const Future &) = delete;
    Future(Future &&) = delete;
    Future &operator=(const Future &) = delete;
    Future &operator=(Future &&) = delete;
    ~Future() = delete;

    template <typename T>
    static FutureSP<T> successful(const T &value) noexcept
    {
        return Future<T>::successful(value);
    }

    template <typename T>
    static FutureSP<QVector<T>> sequence(const QVector<FutureSP<T>> &container) noexcept
    {
        return Future<T>::sequence(container);
    }
};

} // namespace Proof

#endif // PROOF_FUTURE_H
