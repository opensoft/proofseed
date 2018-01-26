#ifndef PROOF_FUTURE_H
#define PROOF_FUTURE_H

#include "proofcore/spinlock.h"
#include "proofcore/proofalgorithms.h"

//TODO: 1.0: think about switching to martinmoene or TartanLlama implementations
#include "proofcore/3rdparty/optional.hpp"

#include <QThread>
#include <QString>
#include <QLinkedList>
#include <QTime>

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
    Failure(const QString &message, long moduleCode, long errorCode, long hints = 0, const QVariant &data = QVariant())
        : exists(true), moduleCode(moduleCode), errorCode(errorCode), hints(hints), message(message), data(data)
    {}
    Failure(const QVariant &data)
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
bool hasLastFailure();
Failure lastFailure();
void resetLastFailure();
void setLastFailure(Failure &&failure);
void setLastFailure(const Failure &failure);
}
}

//This helper struct is designed to be used only in next cases:
//1. return clause of Future::map()/Future::reduce()
//2. Promise::success argument
//3. tasks::run() family
//All other usages will lead to undefined behavior
//Storing and/or casting to T/FutureSP<T> explicitly will lead to undefined behavior
template<typename T, typename F = Failure> struct WithFailure
{
    WithFailure(const F &f) {m_failure = f;}
    template<typename ...Args>
    WithFailure(const Args &...args) {m_failure = F(args...);}
    operator T()
    {
        futures::__util::setLastFailure(std::move(m_failure));
        return T();
    }
    operator FutureSP<T>()
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

    void failure(const Failure &reason)
    {
        m_future->fillFailure(reason);
    }

    void success(T &&result)
    {
        m_future->fillSuccess(result);
    }

private:
    FutureSP<T> m_future = Future<T>::create();
    std::atomic_bool m_filled {false};
};
template<typename T> using PromiseSP = QSharedPointer<Promise<T>>;
template<typename T> using PromiseWP = QWeakPointer<Promise<T>>;

template<typename T>
class Future
{
    template<typename U> friend class Future;
    friend class Promise<T>;
    friend class WithFailure<T>;
public:
    Future(const Future<T> &) = delete;
    Future(Future<T> &&) = delete;
    Future &operator=(const Future<T> &) = delete;
    Future &operator=(Future<T> &&) = delete;
    ~Future() {}

    bool completed() const
    {
        return m_completed;
    }
    bool failed() const
    {
        return m_completed && !m_success;
    }
    bool succeeded() const
    {
        return m_completed && m_success;
    }

    bool wait(long long timeout = -1) const
    {
        QTime timer;
        timer.start();
        while (!m_completed && (timeout < 0 || timer.elapsed() < timeout))
            QThread::yieldCurrentThread();
        return m_completed;
    }

    T result() const
    {
        if (!m_completed)
            wait();
        if (m_success) {
            Q_ASSERT(m_result.has_value());
            return m_result.value();
        }
        return T();
    }

    Failure failureReason() const
    {
        while (!m_completed)
            QThread::yieldCurrentThread();
        if (!m_success) {
            Q_ASSERT(m_failureReason.has_value());
            return m_failureReason.value();
        }
        return Failure();
    }

    template<typename Func>
    auto onSuccess(Func &&f)
    -> decltype(f(T()), FutureSP<T>())
    {
        std::function<void(const T&)> castedF = std::forward<Func>(f);
        bool callIt = false;
        m_mainLock.lock();
        if (m_completed)
            callIt = true;
        else
            m_successCallbacks << castedF;
        m_mainLock.unlock();

        if (callIt && m_success)
            castedF(m_result.value());
        return m_weakSelf.toStrongRef();
    }

    template<typename Func>
    auto onFailure(Func &&f)
    -> decltype(f(Failure()), FutureSP<T>())
    {
        std::function<void(const Failure&)> castedF = std::forward<Func>(f);
        bool callIt = false;
        m_mainLock.lock();
        if (m_completed)
            callIt = true;
        else
            m_failureCallbacks << castedF;
        m_mainLock.unlock();

        if (callIt && !m_success)
            castedF(failureReason());
        return m_weakSelf.toStrongRef();
    }

    template<typename Func>
    auto forEach(Func &&f)
    -> decltype(f(T()), FutureSP<T>())
    {
        return onSuccess(std::move(f));
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

    //This methods were made non-template to restrict usage only to linear qt containers
    static FutureSP<QList<T>> sequence(QList<FutureSP<T>> container)
    {
        PromiseSP<QList<T>> promise = PromiseSP<QList<T>>::create();
        QList<T> result;
        result.reserve(container.count());
        iterateSequence(std::move(container), container.cbegin(), std::move(result), promise);
        return promise->future();
    }

    static FutureSP<QVector<T>> sequence(QVector<FutureSP<T>> container)
    {
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
        Q_ASSERT_X(!m_completed, "Future::fillSuccess", "Can't fill one future twice");
        if (futures::__util::hasLastFailure()) {
            auto failure = futures::__util::lastFailure();
            futures::__util::resetLastFailure();
            fillFailure(failure);
            return;
        }

        m_mainLock.lock();
        m_result = result;
        m_success = true;
        m_completed = true;
        for (const auto &f : qAsConst(m_successCallbacks))
            f(result);
        m_mainLock.unlock();
    }

    void fillFailure(const Failure &reason)
    {
        Q_ASSERT_X(!m_completed, "Future::fillFailure", "Can't fill one future twice");
        m_mainLock.lock();
        m_failureReason = reason;
        m_success = false;
        m_completed = true;
        for (const auto &f : qAsConst(m_failureCallbacks))
            f(reason);
        m_mainLock.unlock();
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
        (*current)->onSuccess(sequenceSuccessListenerGenerator(std::move(initial), current, std::forward<Container<T>>(result), promise));
        (*current)->onFailure([promise](const Failure &reason) {promise->failure(reason);});
    }

    template<typename It, template<typename...> class Container>
    static std::function<void(const T&)> sequenceSuccessListenerGenerator(Container<FutureSP<T>> &&initial, It current,
                                                                          Container<T> &&result, const PromiseSP<Container<T>> &promise)
    {
        return [initial = std::move(initial), current, result = std::forward<Container<T>>(result), promise](const T &v) mutable {
            ++current;
            result << v;
            if (current == initial.cend())
                promise->success(std::move(result));
            else
                iterateSequence(std::move(initial), current, std::move(result), promise);
        };
    }

    std::atomic_bool m_completed {false};
    std::atomic_bool m_success {false};
    std::experimental::optional<T> m_result;
    std::experimental::optional<Failure> m_failureReason;

    QLinkedList<std::function<void(const T&)>> m_successCallbacks;
    QLinkedList<std::function<void(const Failure&)>> m_failureCallbacks;
    SpinLock m_mainLock;
    FutureWP<T> m_weakSelf;
};
}
#endif // PROOF_FUTURE_H
