#ifndef PROOF_FUTURE_H
#define PROOF_FUTURE_H

#include "proofcore/spinlock.h"
#include "proofcore/proofalgorithms.h"

//TODO: 1.0: think about switching to martinmoene or TartanLlama implementations
#include "proofcore/3rdparty/optional.hpp"

#include <QThread>
#include <QString>
#include <QLinkedList>

namespace Proof {
//TODO: add Promise<void>, Future<void> if ever will be needed
template<typename T> class Future;
template<typename T> using FutureSP = QSharedPointer<Future<T>>;
template<typename T> using FutureWP = QWeakPointer<Future<T>>;

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

    void failure(const QString &reason)
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

    T result() const
    {
        while (!m_completed)
            QThread::yieldCurrentThread();
        if (m_success) {
            Q_ASSERT(m_result.has_value());
            return m_result.value();
        }
        return T();
    }

    QString failureReason() const
    {
        while (!m_completed)
            QThread::yieldCurrentThread();
        return m_success ? QLatin1String() : m_failureReason;
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
    -> decltype(f(QString()), FutureSP<T>())
    {
        std::function<void(const QString&)> castedF = std::forward<Func>(f);
        bool callIt = false;
        m_mainLock.lock();
        if (m_completed)
            callIt = true;
        else
            m_failureCallbacks << castedF;
        m_mainLock.unlock();

        if (callIt && !m_success)
            castedF(m_failureReason);
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
        return result;
    }

    template<typename Func>
    auto flatMap(Func &&f)
    -> decltype(FutureSP<decltype(f(T())->result())>())
    {
        using U = decltype(f(T())->result());
        FutureSP<U> result = Future<U>::create();
        onSuccess([result, f = std::forward<Func>(f)](const T &v) { f(v)->onSuccess([result](const U &v){ result->fillSuccess(v); }); });
        return result;
    }

    template<typename Func, typename Result>
    auto reduce(Func &&f, Result acc)
    -> decltype(FutureSP<Result>())
    {
        FutureSP<Result> result = Future<Result>::create();
        onSuccess([result, f = std::forward<Func>(f), acc](const T &v) { result->fillSuccess(algorithms::reduce(v, f, acc)); });
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
        m_mainLock.lock();
        Q_ASSERT_X(!m_completed, "Future::fillSuccess", "Can't fill one future twice");
        m_result = result;
        m_success = true;
        m_completed = true;
        for (const auto &f : qAsConst(m_successCallbacks))
            f(result);
        m_mainLock.unlock();
    }

    void fillFailure(const QString &reason)
    {
        m_mainLock.lock();
        Q_ASSERT_X(!m_completed, "Future::fillFailure", "Can't fill one future twice");
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
        (*current)->onFailure([promise](const QString &reason) {promise->failure(reason);});
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
    QString m_failureReason;
    std::experimental::optional<T> m_result;

    QLinkedList<std::function<void(const T&)>> m_successCallbacks;
    QLinkedList<std::function<void(const QString&)>> m_failureCallbacks;
    SpinLock m_mainLock;
    FutureWP<T> m_weakSelf;
};
}
#endif // PROOF_FUTURE_H
