// clazy:skip

#include "gtest/test_global.h"

#include "proofcore/tasks.h"
#include "proofcore/future.h"
#include "proofcore/proofalgorithms.h"

#include <QThread>
#include <QDateTime>
#include <QTimer>
#include <QPair>

using namespace Proof;
using namespace Proof::tasks;

namespace {
inline quint64 currentThread()
{
    return reinterpret_cast<quint64>(QThread::currentThread());
}
template <typename T> using Result = QPair<quint64, T>;
template<typename T>
Result<T> pairedResult(const T &v)
{
    return qMakePair(currentThread(), v);
}
}

TEST(TasksTest, capacities)
{
    auto dispatcher = TasksDispatcher::instance();
    EXPECT_GE(dispatcher->capacity(), dispatcher->restrictorCapacity(RestrictionType::Intensive));
    EXPECT_GE(dispatcher->capacity(), dispatcher->restrictorCapacity(RestrictionType::Http, "some_unique_restrictor"));
    EXPECT_GE(dispatcher->capacity(), dispatcher->restrictorCapacity(RestrictionType::Custom, "some_unique_restrictor"));
    EXPECT_EQ(dispatcher->capacity(), dispatcher->restrictorCapacity(RestrictionType::Http));
    EXPECT_EQ(dispatcher->capacity(), dispatcher->restrictorCapacity(RestrictionType::Custom));
    EXPECT_NE(2, dispatcher->restrictorCapacity(RestrictionType::Custom, "some_unique_restrictor_2"));
    dispatcher->addCustomRestrictor("some_unique_restrictor_2", 2);
    EXPECT_EQ(2, dispatcher->restrictorCapacity(RestrictionType::Custom, "some_unique_restrictor_2"));
}

TEST(TasksTest, singleTask)
{
    Result<int> result = run([](){return pairedResult(42);})->result();
    EXPECT_NE(currentThread(), result.first);
    EXPECT_EQ(42, result.second);
}

TEST(TasksTest, multipleTasks)
{
    std::atomic_bool ready {false};
    int n = 5;
    QList<FutureSP<Result<int>>> results;
    for (int i = 0; i < n; ++i)
        results << run([&ready, i](){while (!ready); return pairedResult(i * 2);});
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
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    int n = TasksDispatcher::instance()->capacity() * 2;
    QList<FutureSP<int>> results;
    for (int i = 0; i < n; ++i)
        results << run([&ready, &runCounter, i](){++runCounter; while (!ready); return i * 2;});
    QTime timeout;
    timeout.start();
    while (runCounter < TasksDispatcher::instance()->capacity() && timeout.elapsed() < 1000);
    QThread::msleep(25);
    EXPECT_EQ(TasksDispatcher::instance()->capacity(), runCounter);
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(results[i]->completed());
    ready = true;
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, results[i]->result());
}

TEST(TasksTest, multipleHttpTasksOverCapacity)
{
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    std::atomic_int otherRunCounter {0};
    int capacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Http, "test");
    int n = capacity * 2;
    QList<FutureSP<int>> results;
    QList<FutureSP<int>> otherResults;
    for (int i = 0; i < n; ++i) {
        results << run([&ready, &runCounter, i](){++runCounter; while (!ready); return i * 2;}, RestrictionType::Http, "test");
        otherResults << run([&ready, &otherRunCounter, i](){++otherRunCounter; while (!ready); return i * 3;}, RestrictionType::Http, "other");
    }
    QTime timeout;
    timeout.start();
    while ((runCounter < capacity || otherRunCounter < capacity) && timeout.elapsed() < 5000);
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_EQ(capacity, otherRunCounter);
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

TEST(TasksTest, multipleIntensiveTasksOverCapacity)
{
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    int capacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Intensive);
    int n = capacity * 2;
    QList<FutureSP<int>> results;
    for (int i = 0; i < n; ++i)
        results << run([&ready, &runCounter, i](){++runCounter; while (!ready); return i * 2;}, RestrictionType::Intensive);
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000);
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
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    std::atomic_int otherRunCounter {0};
    int capacity = 2;
    TasksDispatcher::instance()->addCustomRestrictor("test", capacity);
    int fullCapacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Custom, "other");
    int n = fullCapacity * 2;
    QList<FutureSP<int>> results;
    QList<FutureSP<int>> otherResults;
    for (int i = 0; i < n; ++i) {
        results << run([&ready, &runCounter, i](){++runCounter; while (!ready); return i * 2;}, RestrictionType::Custom, "test");
        otherResults << run([&ready, &otherRunCounter, i](){++otherRunCounter; while (!ready); return i * 3;}, RestrictionType::Custom, "other");
    }
    QTime timeout;
    timeout.start();
    while ((runCounter < capacity || otherRunCounter < fullCapacity) && timeout.elapsed() < 5000);
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
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Intensive);
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = run(input, [&ready, &runCounter](int x){++runCounter; while (!ready); return x * 2;}, RestrictionType::Intensive);
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000);
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
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->capacity();
    int n = capacity * 2;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = run(input, [&ready, &runCounter](int x){++runCounter; while (!ready); return x * 2;});
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000);
    QThread::msleep(25);
    EXPECT_EQ(capacity, runCounter);
    EXPECT_FALSE(future->completed());
    ready = true;
    auto result = future->result();
    ASSERT_EQ(n, result.count());
    for (int i = 0; i < n; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(TasksTest, clusteredRun)
{
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    SpinLock initialDataLock;
    QList<int> initialData;
    QVector<int> input;
    int capacity = TasksDispatcher::instance()->restrictorCapacity(RestrictionType::Intensive);
    int n = capacity * 100;
    int minClusterSize = 5;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = clusteredRun(input, [&ready, &runCounter, &initialDataLock, &initialData](int x) {
        if (!ready) {
            initialDataLock.lock();
            initialData << x;
            initialDataLock.unlock();
        }
        ++runCounter;
        while (!ready);
        return x * 2;
    }, minClusterSize, RestrictionType::Intensive);
    QTime timeout;
    timeout.start();
    while (runCounter < capacity && timeout.elapsed() < 1000);
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

TEST(TasksTest, clusteredRunWithExtraBigCluster)
{
    std::atomic_bool ready {false};
    std::atomic_int runCounter {0};
    SpinLock initialDataLock;
    QList<int> initialData;
    QVector<int> input;

    int capacity = 4;
    TasksDispatcher::instance()->addCustomRestrictor("extraBigCluster", capacity);
    int n = capacity * 100;
    int realClustersCount = 2;
    int minClusterSize = n / realClustersCount;
    for (int i = 0; i < n; ++i)
        input << i;
    FutureSP<QVector<int>> future = clusteredRun(input, [&ready, &runCounter, &initialDataLock, &initialData](int x) {
        if (!ready) {
            initialDataLock.lock();
            initialData << x;
            initialDataLock.unlock();
        }
        ++runCounter;
        while (!ready);
        return x * 2;
    }, minClusterSize, RestrictionType::Intensive);
    QTime timeout;
    timeout.start();
    while (runCounter < realClustersCount && timeout.elapsed() < 1000);
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
    std::atomic_bool ready {false};
    QTimer *timer = new QTimer();
    timer = new QTimer;
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
    while (!ready);
    EXPECT_FALSE(future->completed());
    QMetaObject::invokeMethod(timer, "start", Qt::BlockingQueuedConnection, Q_ARG(int, 1));
    QTime timeout;
    timeout.start();
    while (!future->completed() && timeout.elapsed() < 1000);
    EXPECT_TRUE(future->completed());
    EXPECT_EQ(42, future->result());
    thread.quit();
    thread.wait(100);
    delete timer;
}
