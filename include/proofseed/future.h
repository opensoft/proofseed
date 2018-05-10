#ifndef PROOF_FUTURE_H
#define PROOF_FUTURE_H

#include "proofcore/proofcore_global.h"
#include "proofcore/spinlock.h"
#include "proofcore/proofalgorithms.h"
#include "proofcore/helpers/tuplemaker.h"
#include "proofcore/helpers/zipper.h"

//TODO: 1.0: think about switching to martinmoene or TartanLlama implementations
#include "proofcore/3rdparty/optional.hpp"

#include <QThread>
#include <QString>
#include <QTime>
#include <QMutex>
#include <QWaitCondition>
#include <QCoreApplication>

#include <list>

namespace Proof {
//TODO: add Promise<void>, Future<void> if ever will be needed
template<typename T> class Future;
template<typename T> using FutureSP = QSharedPointer<Future<T>>;
template<typename T> using FutureWP = QWeakPointer<Future<T>>;

struct Failure
{
    enum Hints {
        NoHint = 0x0,
        UserFriendlyHint = 0x1,
        CriticalHint = 0x2
    };
    Failure(const QString &message, long moduleCode, long errorCode, long hints = Failure::NoHint, const QVariant &data = QVariant())
        : exists(true), moduleCode(moduleCode), errorCode(errorCode), hints(hints), message(message), data(data)
    {}
    explicit Failure(const QVariant &data)
        : exists(true), moduleCode(0), errorCode(0), hints(NoHint), message(QLatin1String()), data(data)
    {}
    Failure() : exists(false), moduleCode(0), errorCode(0), hints(NoHint) {}
    operator QString() {return message;}
    bool exists = false;
    long moduleCode = 0;
    long errorCode = 0;
    long hints = 0;
    QString message;
    QVariant data;
};

namespace futures {
namespace __util {
bool PROOF_CORE_EXPORT hasLastFailure();
Failure PROOF_CORE_EXPORT lastFailure();
void PROOF_CORE_EXPORT resetLastFailure();
void PROOF_CORE_EXPORT setLastFailure(Failure &&failure);
void PROOF_CORE_EXPORT setLastFailure(const Failure &failure);
}
}

//This helper struct is designed to be used only in next cases:
//1. return clause of Future::map()/Future::reduce()
//2. Promise::success argument
//3. tasks::run() family
//All other usages will lead to undefined behavior
//Storing and/or casting to T/FutureSP<T> explicitly will lead to undefined behavior
struct WithFailure
{
    explicit WithFailure(const Failure &f) {m_failure = f;}
    explicit WithFailure(Failure &&f) {m_failure = std::move(f);}
    template<typename ...Args>
    explicit WithFailure(const Args &...args) {m_failure = Failure(args...);}
    template<typename T> operator T()
    {
        futures::__util::setLastFailure(std::move(m_failure));
        return T();
    }
    template<typename T> operator FutureSP<T>()
    {
        FutureSP<T> result = Future<T>::create();
        result->fillFailure(std::move(m_failure));
        return result;
    }
private:
    Failure m_failure;
};

template<typename T>
class Promise
{
public:
    Promise() = default;
    Promise(const Promise<T> &) = delete;
    Promise(Promise<T> &&) = delete;
    Promise &operator=(const Promise<T> &) = delete;
    Promise &operator=(Promise<T> &&) = delete;
    ~Promise() = default;

    FutureSP<T> future() const
    {
        return m_future;
    }

    bool filled()
    {
        return m_future->completed();
    }

    void failure(const Failure &reason)
    {
        m_future->fillFailure(reason);
    }

    void failure(Failure &&reason)
    {
        m_future->fillFailure(reason);
    }

    void success(const T &result)
    {
        m_future->fillSuccess(result);
    }

private:
    FutureSP<T> m_future = Future<T>::create();
};
template<typename T> using PromiseSP = QSharedPointer<Promise<T>>;
template<typename T> using PromiseWP = QWeakPointer<Promise<T>>;

template<typename T>
class CancelableFuture
{
public:
    using Type = T;
    using element_type = T;
    using value_type = T;
    explicit CancelableFuture() {m_promise = PromiseSP<T>::create();}
    explicit CancelableFuture(const PromiseSP<T> &promise) {m_promise = promise;}
    void cancel() const
    {
        if (!m_promise->filled())
            m_promise->failure(Failure("Canceled", 0, 0));
    }
    Future<T> *operator->() const
    {
        return m_promise->future().data();
    }
    operator FutureSP<T>() const
    {
        return m_promise->future();
    }
private:
    PromiseSP<T> m_promise;
};

template<typename T>
class Future
{
    template<typename U> friend class Future;
    friend class Promise<T>;
    friend struct WithFailure;
public:
    using Value = T;

    Future(const Future<T> &) = delete;
    Future(Future<T> &&) = delete;
    Future &operator=(const Future<T> &) = delete;
    Future &operator=(Future<T> &&) = delete;
    ~Future() = default;

    bool completed() const
    {
        int value = m_state.load(std::memory_order_acquire);
        return value == FailedFuture || value == SucceededFuture;
    }
    bool failed() const
    {
        return m_state.load(std::memory_order_acquire) == FailedFuture;
    }
    bool succeeded() const
    {
        return m_state.load(std::memory_order_acquire) == SucceededFuture;
    }

    bool wait(long long timeout = -1) const
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
            self->recover([](const Failure &) {
                return T();
            })->onSuccess([&waiter, &mutex, &wasInSameThread, waitingThread = QThread::currentThread()](const T &) {
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
            QThread::yieldCurrentThread();
        if (failed()) {
            Q_ASSERT(m_failureReason.has_value());
            return m_failureReason.value();
        }
        return Failure();
    }

    template<typename Func>
    auto onSuccess(Func &&f)
    -> decltype(f(T()), FutureSP<T>())
    {
        bool callIt = false;
        m_mainLock.lock();
        if (completed())
            callIt = true;
        else
            m_successCallbacks.emplace_back(f);
        m_mainLock.unlock();

        if (callIt && succeeded())
            f(m_result.value());
        return m_weakSelf.toStrongRef();
    }

    template<typename Func>
    auto onFailure(Func &&f)
    -> decltype(f(Failure()), FutureSP<T>())
    {
        bool callIt = false;
        m_mainLock.lock();
        if (completed())
            callIt = true;
        else
            m_failureCallbacks.emplace_back(f);
        m_mainLock.unlock();

        if (callIt && failed())
            f(failureReason());
        return m_weakSelf.toStrongRef();
    }

    template<typename Func>
    auto forEach(Func &&f)
    -> decltype(f(T()), FutureSP<T>())
    {
        return onSuccess(std::forward<Func>(f));
    }

    template<typename Func>
    auto filter(Func &&f, const Failure &rejected = Failure(QStringLiteral("Result wasn't good enough"), 0, 0))
    -> decltype(!!f(T()), FutureSP<T>())
    {
        FutureSP<T> result = Future<T>::create();
        onSuccess([result, f = std::forward<Func>(f), rejected](const T &v) {
            if (f(v))
                result->fillSuccess(v);
            else
                result->fillFailure(rejected);
        });
        onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        return result;
    }

    template<typename Func>
    auto map(Func &&f)
    -> decltype(FutureSP<decltype(f(T()))>())
    {
        using U = decltype(f(T()));
        FutureSP<U> result = Future<U>::create();
        onSuccess([result, f = std::forward<Func>(f)](const T &v) { result->fillSuccess(f(v)); });
        onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        return result;
    }

    template<typename Func>
    auto flatMap(Func &&f)
    -> decltype(FutureSP<decltype(f(T())->result())>())
    {
        using U = decltype(f(T())->result());
        FutureSP<U> result = Future<U>::create();
        onSuccess([result, f = std::forward<Func>(f)](const T &v) {
            FutureSP<U> inner = f(v);
            inner->onSuccess([result](const U &v){ result->fillSuccess(v); });
            inner->onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        });
        onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        return result;
    }

    template<typename Func>
    auto andThen(Func &&f)
    -> decltype(FutureSP<decltype(f()->result())>())
    {
        using U = decltype(f()->result());
        FutureSP<U> result = Future<U>::create();
        onSuccess([result, f = std::forward<Func>(f)](const T &) {
            FutureSP<U> inner = f();
            inner->onSuccess([result](const U &v){ result->fillSuccess(v); });
            inner->onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        });
        onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        return result;
    }

    template<typename T2,
             typename Result = typename std::decay<T2>::type>
    FutureSP<Result> andThenValue(T2 &&value)
    {
        return andThen([value = std::forward<T2>(value)]() {
            return Future<Result>::successful(value);
        });
    }

    template<typename Func, typename Result>
    auto reduce(Func &&f, Result acc)
    -> decltype(FutureSP<Result>())
    {
        FutureSP<Result> result = Future<Result>::create();
        onSuccess([result, f = std::forward<Func>(f), acc](const T &v) { result->fillSuccess(algorithms::reduce(v, f, acc)); });
        onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        return result;
    }

    template<typename Func>
    auto recover(Func &&f)
    -> decltype(f(Failure()), FutureSP<T>())
    {
        FutureSP<T> result = Future<T>::create();
        onSuccess([result](const T &v) { result->fillSuccess(v); });
        onFailure([result, f = std::forward<Func>(f)](const Failure &failure) { result->fillSuccess(f(failure)); });
        return result;
    }

    template<typename Func>
    auto recoverWith(Func &&f)
    -> decltype(f(Failure())->result(), FutureSP<T>())
    {
        FutureSP<T> result = Future<T>::create();
        onSuccess([result](const T &v) { result->fillSuccess(v); });
        onFailure([result, f = std::forward<Func>(f)](const Failure &failure) {
            FutureSP<T> inner = f(failure);
            inner->onSuccess([result](const T &v){ result->fillSuccess(v); });
            inner->onFailure([result](const Failure &failure) { result->fillFailure(failure); });
        });
        return result;
    }

    //TODO: think about better approach for zip. TupleMaker seems a bit sloppy
    template<typename Head, typename ...Other,
             typename ResultSP = typename __util::ZipperSP<std::true_type, FutureSP<T>, Head, Other...>::type,
             typename Result = typename std::decay<decltype(*ResultSP().data())>::type::Value>
    FutureSP<Result> zip(Head head, Other... other)
    {
        return flatMap([head, other...](const T &v) -> FutureSP<Result> {
            return head->zip(other...)->map([v](const auto &argsResult) -> Result {
                return std::tuple_cat(__util::TupleMaker<T>::result(v), argsResult);
            });
        });
    }

    static auto successful()
    -> decltype(T(), FutureSP<T>())
    {
        FutureSP<T> result = create();
        result->fillSuccess(T());
        return result;
    }

    static FutureSP<T> successful(const T &value)
    {
        FutureSP<T> result = create();
        result->fillSuccess(value);
        return result;
    }

    static FutureSP<T> fail(const Failure &failure)
    {
        FutureSP<T> result = create();
        result->fillFailure(failure);
        return result;
    }

    //This method was made non-template to restrict usage only to linear qt containers because of container copying
    static FutureSP<QList<T>> sequence(QList<FutureSP<T>> container)
    {
        if (container.isEmpty())
            return Future<QList<T>>::successful();
        PromiseSP<QList<T>> promise = PromiseSP<QList<T>>::create();
        QList<T> result;
        result.reserve(container.count());
        iterateSequence(std::move(container), container.cbegin(), std::move(result), promise);
        return promise->future();
    }

    static FutureSP<QVector<T>> sequence(QVector<FutureSP<T>> container)
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
    Future() = default;
    static FutureSP<T> create()
    {
        //in-place creation is a trade off for future creation restriction
        FutureSP<T> result(new Future<T>);
        result->m_weakSelf = result.toWeakRef();
        return result;
    }

    void fillSuccess(const T &result)
    {
        if (futures::__util::hasLastFailure()) {
            auto failure = futures::__util::lastFailure();
            futures::__util::resetLastFailure();
            fillFailure(failure);
            return;
        }

        m_mainLock.lock();
        if (completed()) {
            qCDebug(proofCoreFuturesLog) << "Trying to fill same future twice. Nothing happens";
            m_mainLock.unlock();
            return;
        }
        m_result = result;
        m_state.store(SucceededFuture, std::memory_order_release);
        for (const auto &f : m_successCallbacks)
            f(result);
        m_successCallbacks.clear();
        m_failureCallbacks.clear();
        m_mainLock.unlock();
    }

    void fillFailure(const Failure &reason)
    {
        m_mainLock.lock();
        if (completed()) {
            qCDebug(proofCoreFuturesLog) << "Trying to fill same future twice. Nothing happens";
            m_mainLock.unlock();
            return;
        }
        m_failureReason = reason;
        m_state.store(FailedFuture, std::memory_order_release);
        for (const auto &f : m_failureCallbacks)
            f(reason);
        m_successCallbacks.clear();
        m_failureCallbacks.clear();
        m_mainLock.unlock();
    }

    auto zip() -> decltype(FutureSP<decltype(__util::TupleMaker<T>::result(T()))>())
    {
        return map([](const T &v) {return __util::TupleMaker<T>::result(v);});
    }

    template<typename It, template<typename...> class Container>
    static void iterateSequence(Container<FutureSP<T>> &&initial, It current,
                                Container<T> &&result, const PromiseSP<Container<T>> &promise)
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
        currentFuture->onSuccess(sequenceSuccessListenerGenerator(std::move(initial), current, std::move(result), promise));
        currentFuture->onFailure([promise](const Failure &reason) {promise->failure(reason);});
    }

    template<typename It, template<typename...> class Container>
    static std::function<void(const T&)> sequenceSuccessListenerGenerator(Container<FutureSP<T>> &&initial, It current,
                                                                          Container<T> &&result, const PromiseSP<Container<T>> &promise)
    {
        return [initial = std::move(initial), current, result = std::move(result), promise](const T &v) mutable {
            ++current;
            result << v;
            if (current == initial.cend())
                promise->success(std::move(result));
            else
                iterateSequence(std::move(initial), current, std::move(result), promise);
        };
    }

    enum State {
        NotCompletedFuture = 0,
        SucceededFuture = 1,
        FailedFuture = 2
    };

    std::atomic_int m_state {NotCompletedFuture};
    std::experimental::optional<T> m_result;
    std::experimental::optional<Failure> m_failureReason;

    std::list<std::function<void(const T&)>> m_successCallbacks;
    std::list<std::function<void(const Failure&)>> m_failureCallbacks;
    SpinLock m_mainLock;
    FutureWP<T> m_weakSelf;
};
}

#endif // PROOF_FUTURE_H
