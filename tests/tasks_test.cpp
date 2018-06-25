// clazy:skip

#include "proofcore/future.h"
#include "proofcore/proofalgorithms.h"
#include "proofcore/tasks.h"

#include <QDateTime>
#include <QPair>
#include <QThread>
#include <QTimer>

#include "gtest/test_global.h"

using namespace Proof;
using namespace Proof::tasks;

namespace {
inline quint64 currentThread()
{
    return reinterpret_cast<quint64>(QThread::currentThread());
}
template <typename T>
using Result = QPair<quint64, T>;
template <typename T>
Result<T> pairedResult(const T &v)
{
    return qMakePair(currentThread(), v);
}
} // namespace

TEST(TasksTest, capacities)
{
    auto dispatcher = TasksDispatcher::instance();
    EXPECT_GE(dispatcher->capacity(), dispatcher->restrictorCapacity(RestrictionType::Intensive));
    EXPECT_GE(dispatcher->capacity(),
              dispatcher->restrictorCapacity(RestrictionType::Custom, "some_unique_restrictor"));
    EXPECT_EQ(dispatcher->capacity(), dispatcher->restrictorCapacity(RestrictionType::Custom));
    EXPECT_NE(2, dispatcher->restrictorCapacity(RestrictionType::Custom, "some_unique_restrictor_2"));
    dispatcher->addCustomRestrictor("some_unique_restrictor_2", 2);
    EXPECT_EQ(2, dispatcher->restrictorCapacity(RestrictionType::Custom, "some_unique_restrictor_2"));
    EXPECT_EQ(1, dispatcher->restrictorCapacity(RestrictionType::ThreadBound));
}

TEST(TasksTest, singleTask)
{
    Result<int> result = run([]() { return pairedResult(42); })->result();
    EXPECT_NE(currentThread(), result.first);
    EXPECT_EQ(42, result.second);
}

TEST(TasksTest, singleDeferredTask)
{
    PromiseSP<int> innerPromise = PromiseSP<int>::create();
    FutureSP<int> result = run([innerPromise]() { return innerPromise->future(); });
    EXPECT_FALSE(result->completed());
    innerPromise->success(42);
    result->wait(1000);
    ASSERT_TRUE(result->completed());
    EXPECT_TRUE(result->succeeded());
    EXPECT_FALSE(result->failed());
    EXPECT_EQ(42, result->result());
}

TEST(TasksTest, singleVoidTask)
{
    std::atomic_bool flag{false};
    FutureSP<bool> result = run([&flag]() { flag = true; });
    result->wait(1000);
    ASSERT_TRUE(result->completed());
    EXPECT_TRUE(result->succeeded());
    EXPECT_FALSE(result->failed());
    EXPECT_EQ(true, result->result());
    EXPECT_EQ(true, flag);
}

TEST(TasksTest, taskCancelation)
{
    tasks::TasksDispatcher::instance()->addCustomRestrictor("single", 1);
    PromiseSP<int> blockingPromise = PromiseSP<int>::create();
    run([blockingPromise]() { blockingPromise->future()->wait(); }, RestrictionType::Custom, "single");
    std::atomic_bool executed{false};
    CancelableFuture<int> f = run(
        [&executed]() {
            executed = true;
            return 42;
        },
        RestrictionType::Custom, "single");
    CancelableFuture<int> f2 = run([]() { return 42; }, RestrictionType::Custom, "single");
    f.cancel();
    blockingPromise->success(1);
    f->wait(1000);
    ASSERT_TRUE(f->completed());
    f2->wait(1000);
    ASSERT_TRUE(f2->completed());
    EXPECT_EQ(42, f2->result());

    EXPECT_FALSE(executed);
    EXPECT_FALSE(f->succeeded());
    EXPECT_TRUE(f->failed());
    EXPECT_EQ("Canceled", f->failureReason().message);
}

TEST(TasksTest, deferedTaskCancelation)
{
    tasks::TasksDispatcher::instance()->addCustomRestrictor("single", 1);
    PromiseSP<int> blockingPromise = PromiseSP<int>::create();
    run([blockingPromise]() { return blockingPromise->future()->wait(); }, RestrictionType::Custom, "single");
    std::atomic_bool executed{false};
    PromiseSP<int> innerPromise = PromiseSP<int>::create();
    CancelableFuture<int> f = run(
        [innerPromise, &executed]() {
            executed = true;
            return innerPromise->future();
        },
        RestrictionType::Custom, "single");
    CancelableFuture<int> f2 = run([]() { return 42; }, RestrictionType::Custom, "single");
    f.cancel();
    blockingPromise->success(1);
    f->wait(1000);
    ASSERT_TRUE(f->completed());
    f2->wait(1000);
    ASSERT_TRUE(f2->completed());
    EXPECT_EQ(42, f2->result());

    EXPECT_FALSE(executed);
    EXPECT_FALSE(f->succeeded());
    EXPECT_TRUE(f->failed());
    EXPECT_EQ("Canceled", f->failureReason().message);
}

TEST(TasksTest, voidTaskCancelation)
{
    tasks::TasksDispatcher::instance()->addCustomRestrictor("single", 1);
    PromiseSP<int> blockingPromise = PromiseSP<int>::create();
    run([blockingPromise]() { return blockingPromise->future()->wait(); }, RestrictionType::Custom, "single");
    std::atomic_bool executed{false};
    CancelableFuture<bool> f = run([&executed]() { executed = true; }, RestrictionType::Custom, "single");
    CancelableFuture<int> f2 = run([]() { return 42; }, RestrictionType::Custom, "single");
    f.cancel();
    blockingPromise->success(1);
    f->wait(1000);
    ASSERT_TRUE(f->completed());
    f2->wait(1000);
    ASSERT_TRUE(f2->completed());
    EXPECT_EQ(42, f2->result());

    EXPECT_FALSE(executed);
    EXPECT_FALSE(f->succeeded());
    EXPECT_TRUE(f->failed());
    EXPECT_EQ("Canceled", f->failureReason().message);
}

TEST(TasksTest, singleDeferredTaskWithFailure)
{
    PromiseSP<int> innerPromise = PromiseSP<int>::create();
    FutureSP<int> result = run([innerPromise]() { return innerPromise->future(); });
    EXPECT_FALSE(result->completed());
    innerPromise->failure(Failure("failed", 0, 0));
    result->wait(1000);
    ASSERT_TRUE(result->completed());
    EXPECT_FALSE(result->succeeded());
    EXPECT_TRUE(result->failed());
    EXPECT_EQ(0, result->result());
    EXPECT_EQ("failed", result->failureReason().message);
}

TEST(TasksTest, multipleTasks)
{
    std::atomic_bool ready{false};
    int n = 5;
    QVector<FutureSP<Result<int>>> results;
    for (int i = 0; i < n; ++i) {
        results << run([&ready, i]() {
            while (!ready)
                ;
            return pairedResult(i * 2);
        });
    }
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(results[i]->completed());
    ready = true;
    QSet<quint64> threads;
    for (int i = 0; i < n; ++i) {
        auto result = results[i]->result();
        threads << result.first;
        EXPECT_NE(currentThread(), result.first);
        EXPECT_EQ(i * 2, result.second);
    }
    EXPECT_EQ(n, threads.count());
}

TEST(TasksTest, multipleTasksOverCapacity)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    int n = TasksDispatcher::instance()->capacity() * 2;
    QVector<FutureSP<int>> results;
    for (int i = 0; i < n; ++i) {
        results << run([&ready, &runCounter, i]() {
            ++runCounter;
            while (!ready)
                ;
            return i * 2;
        });
    }
    QTime timeout;
    timeout.start();
    while (runCounter < TasksDispatcher::instance()->capacity() && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(TasksDispatcher::instance()->capacity(), runCounter);
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(results[i]->completed());
    ready = true;
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, results[i]->result());
}

TEST(TasksTest, multipleIntensiveTasksOverCapacity)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    int capacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Intensive);
    int n = capacity * 2;
    QVector<FutureSP<int>> results;
    for (int i = 0; i < n; ++i) {
        results << run(
            [&ready, &runCounter, i]() {
                ++runCounter;
                while (!ready)
                    ;
                return i * 2;
            },
            RestrictionType::Intensive);
    }
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(results[i]->completed());
    ready = true;
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, results[i]->result());
}

TEST(TasksTest, multipleCustomTasksOverCapacity)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    std::atomic_int otherRunCounter{0};
    int capacity = 2;
    TasksDispatcher::instance()->addCustomRestrictor("test", capacity);
    int fullCapacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Custom, "other");
    int n = fullCapacity * 2;
    QVector<FutureSP<int>> results;
    QVector<FutureSP<int>> otherResults;
    for (int i = 0; i < n; ++i) {
        results << run(
            [&ready, &runCounter, i]() {
                ++runCounter;
                while (!ready)
                    ;
                return i * 2;
            },
            RestrictionType::Custom, "test");
        otherResults << run(
            [&ready, &otherRunCounter, i]() {
                ++otherRunCounter;
                while (!ready)
                    ;
                return i * 3;
            },
            RestrictionType::Custom, "other");
    }
    QTime timeout;
    timeout.start();
    while ((runCounter < capacity || otherRunCounter < fullCapacity) && timeout.elapsed() < 5000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_EQ(fullCapacity, otherRunCounter);
    for (int i = 0; i < n; ++i) {
        EXPECT_FALSE(results[i]->completed());
        EXPECT_FALSE(otherResults[i]->completed());
    }
    ready = true;
    for (int i = 0; i < n; ++i) {
        EXPECT_EQ(i * 2, results[i]->result());
        EXPECT_EQ(i * 3, otherResults[i]->result());
    }
}

TEST(TasksTest, restrictedSequenceRun)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Intensive);
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = run(input,
                                        [&ready, &runCounter](int x) {
                                            ++runCounter;
                                            while (!ready)
                                                ;
                                            return x * 2;
                                        },
                                        RestrictionType::Intensive);
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(TasksTest, sequenceRun)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->capacity();
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = run(input, [&ready, &runCounter](int x) {
        ++runCounter;
        while (!ready)
            ;
        return x * 2;
    });
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(TasksTest, deferredSequenceRun)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->capacity();
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = run(input, [&ready, &runCounter](int x) {
        ++runCounter;
        while (!ready)
            ;
        return Future<int>::successful(x * 2);
    });
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(TasksTest, voidSequenceRun)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->capacity();
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<bool> future = run(input, [&ready, &runCounter](int) {
        ++runCounter;
        while (!ready)
            ;
    });
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    EXPECT_TRUE(result);
}

TEST(TasksTest, sequenceRunWithIndices)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->capacity();
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << (n - i);
    FutureSP<QVector<long long>> future = run(input, [n, &ready, &runCounter](long long index, int x) {
        ++runCounter;
        while (!ready)
            ;
        return ((n - index) == x) ? index * 2 : -42;
    });
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(TasksTest, deferredSequenceRunWithIndices)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->capacity();
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << (n - i);
    FutureSP<QVector<long long>> future = run(input, [n, &ready, &runCounter](long long index, int x) {
        ++runCounter;
        while (!ready)
            ;
        return Future<long long>::successful(((n - index) == x) ? index * 2 : -42);
    });
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(TasksTest, voidSequenceRunWithIndices)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->capacity();
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << (n - i);
    FutureSP<bool> future = run(input, [&ready, &runCounter](long long, int) {
        ++runCounter;
        while (!ready)
            ;
    });
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    EXPECT_TRUE(result);
}

TEST(TasksTest, emptySequenceRun)
{
    FutureSP<QVector<int>> future = run(QVector<int>(), [](int x) { return x * 2; });
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    EXPECT_EQ(0, result.count());
}

TEST(TasksTest, clusteredRun)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    SpinLock initialDataLock;
    QVector<int> initialData;
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Intensive);
    int n = capacity * 100;
    int minClusterSize = 5;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = clusteredRun(input,
                                                 [&ready, &runCounter, &initialDataLock, &initialData](int x) {
                                                     if (!ready) {
                                                         initialDataLock.lock();
                                                         initialData << x;
                                                         initialDataLock.unlock();
                                                     }
                                                     ++runCounter;
                                                     while (!ready)
                                                         ;
                                                     return x * 2;
                                                 },
                                                 minClusterSize, RestrictionType::Intensive);
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);

    std::sort(initialData.begin(), initialData.end());
    EXPECT_EQ(capacity, initialData.count());
    for (int i = 1; i < initialData.count(); ++i)
        EXPECT_LE(minClusterSize, initialData[i] - initialData[i - 1]);
}

TEST(TasksTest, emptyClusteredRun)
{
    FutureSP<QVector<int>> future = clusteredRun(QVector<int>(), [](int x) { return x * 2; });
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    auto result = future->result();
    EXPECT_EQ(0, result.count());
}

TEST(TasksTest, clusteredRunWithExtraBigCluster)
{
    std::atomic_bool ready{false};
    std::atomic_int runCounter{0};
    SpinLock initialDataLock;
    QVector<int> initialData;
    QVector<int> input;

    int capacity = 4;
    TasksDispatcher::instance()->addCustomRestrictor("extraBigCluster", capacity);
    int n = capacity * 100;
    int realClustersCount = 2;
    int minClusterSize = n / realClustersCount;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = clusteredRun(input,
                                                 [&ready, &runCounter, &initialDataLock, &initialData](int x) {
                                                     if (!ready) {
                                                         initialDataLock.lock();
                                                         initialData << x;
                                                         initialDataLock.unlock();
                                                     }
                                                     ++runCounter;
                                                     while (!ready)
                                                         ;
                                                     return x * 2;
                                                 },
                                                 minClusterSize, RestrictionType::Intensive);
    QTime timeout;
    timeout.start();
    while (runCounter < realClustersCount && timeout.elapsed() < 1000)
        ;
    QThread::msleep(25);
    EXPECT_EQ(realClustersCount, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);

    std::sort(initialData.begin(), initialData.end());
    EXPECT_EQ(realClustersCount, initialData.count());
    for (int i = 1; i < initialData.count(); ++i)
        EXPECT_EQ(minClusterSize, initialData[i] - initialData[i - 1]);
}

TEST(TasksTest, signalWaiting)
{
    QThread thread;
    thread.start();
    std::atomic_bool ready{false};
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);
    timer->moveToThread(&thread);
    FutureSP<int> future = run([timer, &ready]() {
        int result = 0;
        std::function<bool()> timerCallback = [&result]() {
            result = 42;
            return true;
        };
        addSignalWaiter(timer, &QTimer::timeout, timerCallback);
        ready = true;
        fireSignalWaiters();
        return result;
    });
    while (!ready)
        ;
    EXPECT_FALSE(future->completed());
    QMetaObject::invokeMethod(timer, "start", Qt::BlockingQueuedConnection, Q_ARG(int, 1));
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    EXPECT_EQ(42, future->result());
    thread.quit();
    thread.wait(100);
    delete timer;
}

TEST(TasksTest, multipleTasksWithFailure)
{
    std::atomic_bool ready{false};
    int n = 5;
    QVector<FutureSP<Result<int>>> results;
    for (int i = 0; i < n; ++i) {
        results << run([&ready, i]() -> Result<int> {
            while (!ready)
                ;
            if (i % 2)
                return WithFailure("failed", 0, 0);
            else
                return pairedResult(i * 2);
        });
    }
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(results[i]->completed());
    ready = true;
    for (int i = 0; i < n; ++i) {
        results[i]->wait(1000);
        ASSERT_TRUE(results[i]->completed()) << i;
        if (i % 2) {
            ASSERT_TRUE(results[i]->failed()) << i;
            EXPECT_EQ("failed", results[i]->failureReason().message) << i;
            EXPECT_EQ(0ull, results[i]->result().first) << i;
            EXPECT_EQ(0, results[i]->result().second) << i;
        } else {
            ASSERT_TRUE(results[i]->succeeded()) << i;
            auto result = results[i]->result();
            EXPECT_NE(currentThread(), result.first) << i;
            EXPECT_EQ(i * 2, result.second) << i;
        }
    }
}

TEST(TasksTest, sequenceRunWithFailure)
{
    std::atomic_bool ready{false};
    QVector<int> input;
    const int n = 5;
    for (int i = 0; i < n; ++i)
        input << i;
    std::atomic_int doneCount{0};
    FutureSP<QVector<int>> future = run(input, [&ready, &doneCount](int x) -> int {
        while (!ready)
            ;
        ++doneCount;
        if (x == 3)
            return WithFailure("failed", 0, 0);
        return x * 2;
    });
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->failed());
    EXPECT_EQ("failed", future->failureReason().message);
    EXPECT_EQ(0, future->result().count());
    QTime timeout;
    timeout.start();
    while (doneCount < n && timeout.elapsed() < 1000)
        ;
    EXPECT_EQ(n, doneCount);
}

TEST(TasksTest, clusteredRunWithFailure)
{
    std::atomic_bool ready{false};
    QVector<int> input;
    int n = 20;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = clusteredRun(input,
                                                 [&ready](int x) -> int {
                                                     while (!ready)
                                                         ;
                                                     if (x == 3)
                                                         return WithFailure("failed", 0, 0);
                                                     return x * 2;
                                                 },
                                                 5);
    ready = true;
    future->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->failed());
    EXPECT_EQ("failed", future->failureReason().message);
    EXPECT_EQ(0, future->result().count());
}

TEST(TasksTest, mappedTaskWithFailure)
{
    std::atomic_bool ready{false};
    FutureSP<int> future = run([&ready]() {
        while (!ready)
            ;
        return 42;
    });
    FutureSP<int> mappedFuture = future->map([](int x) -> int { return WithFailure(x); });
    EXPECT_FALSE(future->completed());
    ready = true;
    mappedFuture->wait(1000);
    ASSERT_TRUE(future->completed());
    ASSERT_TRUE(future->succeeded());
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    ASSERT_TRUE(mappedFuture->failed());
    EXPECT_EQ(0, mappedFuture->result());
    EXPECT_EQ(42, mappedFuture->failureReason().data);
}

TEST(TasksTest, threadBinding)
{
    std::atomic_bool firstCanStart{false};
    std::atomic_bool secondStarted{false};

    auto first = [&firstCanStart]() {
        while (!firstCanStart)
            ;
        return pairedResult(1);
    };

    auto second = [&secondStarted]() {
        secondStarted = true;
        return pairedResult(1);
    };

    auto firstResult = run(first, RestrictionType::ThreadBound, "my");
    auto secondResult = run(second, RestrictionType::ThreadBound, "my");
    EXPECT_FALSE(firstResult->completed());
    EXPECT_FALSE(secondResult->completed());
    EXPECT_FALSE(secondStarted);
    firstCanStart = true;
    firstResult->wait();
    secondResult->wait();
    EXPECT_TRUE(secondStarted);
    EXPECT_NE(currentThread(), firstResult->result().first);
    EXPECT_NE(currentThread(), secondResult->result().first);
    EXPECT_EQ(secondResult->result().first, firstResult->result().first);
}

TEST(TasksTest, threadBindingToDifferentKeys)
{
    auto task = []() { return pairedResult(1); };

    QVector<FutureSP<Result<int>>> firstResults;
    QVector<FutureSP<Result<int>>> secondResults;
    int n = TasksDispatcher::instance()->capacity() * 2;
    for (int i = 0; i < n; ++i) {
        firstResults << run(task, RestrictionType::ThreadBound, "my1");
        secondResults << run(task, RestrictionType::ThreadBound, "my2");
    }

    unsigned long long firstThread = 0ull;
    unsigned long long secondThread = 0ull;
    for (const auto &r : firstResults) {
        r->wait();
        if (!firstThread)
            firstThread = r->result().first;
        EXPECT_NE(currentThread(), r->result().first);
        EXPECT_EQ(firstThread, r->result().first);
    }
    for (const auto &r : secondResults) {
        r->wait();
        if (!secondThread)
            secondThread = r->result().first;
        EXPECT_NE(currentThread(), r->result().first);
        EXPECT_EQ(secondThread, r->result().first);
    }
    EXPECT_NE(firstThread, secondThread);
}

TEST(TasksTest, threadBindingAmongNormalTasks)
{
    auto task = []() { return pairedResult(1); };

    QVector<FutureSP<Result<int>>> boundResults;
    QVector<FutureSP<Result<int>>> otherResults;
    int n = TasksDispatcher::instance()->capacity() * 10;
    for (int i = 0; i < n; ++i) {
        if (i % 5)
            otherResults << run(task);
        else
            boundResults << run(task, RestrictionType::ThreadBound, "my3");
    }

    EXPECT_NE(0, boundResults.count());
    EXPECT_NE(0, otherResults.count());
    unsigned long long boundThread = 0ull;
    for (const auto &r : boundResults) {
        r->wait();
        if (!boundThread)
            boundThread = r->result().first;
        EXPECT_NE(currentThread(), r->result().first);
        EXPECT_EQ(boundThread, r->result().first);
    }
    for (const auto &r : otherResults)
        r->wait();
}

TEST(TasksTest, threadBindingToDifferentKeysAmongOtherTasks)
{
    auto task = []() { return pairedResult(1); };

    QVector<FutureSP<Result<int>>> firstResults;
    QVector<FutureSP<Result<int>>> secondResults;
    QVector<FutureSP<Result<int>>> otherResults;
    int n = TasksDispatcher::instance()->capacity() * 20;
    for (int i = 0; i < n; ++i) {
        if (i % 5)
            otherResults << run(task);
        else if (i % 10)
            firstResults << run(task, RestrictionType::ThreadBound, "my4");
        else
            secondResults << run(task, RestrictionType::ThreadBound, "my5");
    }

    EXPECT_NE(0, firstResults.count());
    EXPECT_NE(0, secondResults.count());
    EXPECT_NE(0, otherResults.count());
    unsigned long long firstThread = 0ull;
    unsigned long long secondThread = 0ull;
    for (const auto &r : firstResults) {
        r->wait();
        if (!firstThread)
            firstThread = r->result().first;
        EXPECT_NE(currentThread(), r->result().first);
        EXPECT_EQ(firstThread, r->result().first);
    }
    for (const auto &r : secondResults) {
        r->wait();
        if (!secondThread)
            secondThread = r->result().first;
        EXPECT_NE(currentThread(), r->result().first);
        EXPECT_EQ(secondThread, r->result().first);
    }
    EXPECT_NE(firstThread, secondThread);
    for (const auto &r : otherResults)
        r->wait();
}
