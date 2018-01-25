// clazy:skip

#include "gtest/test_global.h"

#include "proofcore/future.h"
#include "proofcore/proofalgorithms.h"

#include <QThread>
#include <QDateTime>
#include <QTimer>

using namespace Proof;

TEST(FutureTest, success)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->success(42);
    EXPECT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, future->result());
}

TEST(FutureTest, failure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->failure("failed");
    EXPECT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_EQ("failed", future->failureReason());
}

TEST(FutureTest, onSuccess)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result = 0;
    FutureSP<int> futureWithCallback = future->onSuccess([&result](int x){result = x;});
    EXPECT_EQ(future, futureWithCallback);
    promise->success(42);
    EXPECT_EQ(42, result);
}

TEST(FutureTest, onFailure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    QString result;
    FutureSP<int> futureWithCallback = future->onFailure([&result](QString x){result = x;});
    EXPECT_EQ(future, futureWithCallback);
    promise->failure("failed");
    EXPECT_EQ("failed", result);
}

TEST(FutureTest, multipleOnSuccess)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result[3] = {0, 0, 0};
    QString failedResult[3] = {0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        FutureSP<int> futureWithCallback = future->onSuccess([&result = result[i]](int x){result = x;});
        EXPECT_EQ(future, futureWithCallback);
        futureWithCallback = future->onFailure([&failedResult = failedResult[i]](QString x){failedResult = x;});
        EXPECT_EQ(future, futureWithCallback);
    }
    promise->success(42);
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(42, result[i]);
        EXPECT_TRUE(failedResult[i].isEmpty());
    }
}

TEST(FutureTest, multipleOnFailure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result[3] = {0, 0, 0};
    QString failedResult[3] = {0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        FutureSP<int> futureWithCallback = future->onSuccess([&result = result[i]](int x){result = x;});
        EXPECT_EQ(future, futureWithCallback);
        futureWithCallback = future->onFailure([&failedResult = failedResult[i]](QString x){failedResult = x;});
        EXPECT_EQ(future, futureWithCallback);
    }
    promise->failure("failed");
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(0, result[i]);
        EXPECT_EQ("failed", failedResult[i]);
    }
}

TEST(FutureTest, forEach)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result = 0;
    FutureSP<int> futureWithCallback = future->forEach([&result](int x){result = x;});
    EXPECT_EQ(future, futureWithCallback);
    promise->success(42);
    EXPECT_EQ(42, result);
}

TEST(FutureTest, forEachNegative)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result = 0;
    FutureSP<int> futureWithCallback = future->forEach([&result](int x){result = x;});
    EXPECT_EQ(future, futureWithCallback);
    promise->failure("failed");
    EXPECT_EQ(0, result);
}

TEST(FutureTest, map)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->map([](int x){return x * 2;});
    EXPECT_FALSE(mappedFuture->completed());
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    EXPECT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(84, mappedFuture->result());
}

TEST(FutureTest, flatMap)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    PromiseSP<int> innerPromise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->flatMap([innerPromise](int x) {
        return innerPromise->future()->map([x](int y){return x * y;});
    });
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(mappedFuture->completed());
    innerPromise->success(2);
    EXPECT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(84, mappedFuture->result());
}

TEST(FutureTest, differentTypeMap)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<double> mappedFuture = future->map([](int x) -> double {return x / 2.0;});
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    EXPECT_EQ(42, future->result());
    EXPECT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_DOUBLE_EQ(21.0, mappedFuture->result());
}

TEST(FutureTest, differentTypeFlatMap)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    PromiseSP<double> innerPromise = PromiseSP<double>::create();
    FutureSP<int> future = promise->future();
    FutureSP<double> mappedFuture = future->flatMap([innerPromise](int x) {
        return innerPromise->future()->map([x](double y){return x / y;});
    });
    promise->success(42);
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(mappedFuture->completed());
    innerPromise->success(2.0);
    EXPECT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_DOUBLE_EQ(21.0, mappedFuture->result());
}

TEST(FutureTest, reduce)
{
    PromiseSP<QList<int>> promise = PromiseSP<QList<int>>::create();
    FutureSP<QList<int>> future = promise->future();
    FutureSP<int> reducedFuture = future->reduce([](int acc, int x){return acc + x;}, 0);
    EXPECT_FALSE(reducedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    QList<int> result = future->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i + 1, result[i]);
    EXPECT_TRUE(reducedFuture->completed());
    EXPECT_TRUE(reducedFuture->succeeded());
    EXPECT_FALSE(reducedFuture->failed());
    EXPECT_EQ(15, reducedFuture->result());
}

TEST(FutureTest, sequenceQList)
{
    int n = 5;
    QList<PromiseSP<int>> promises;
    for (int i = 0; i < n; ++i)
        promises << PromiseSP<int>::create();
    QList<FutureSP<int>> futures = algorithms::map(promises, [](auto p){return p->future();}, QList<FutureSP<int>>());
    FutureSP<QList<int>> sequencedFuture = Future<int>::sequence(futures);
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(futures[i]->completed());

    for (int i = 0; i < n; ++i) {
        EXPECT_FALSE(sequencedFuture->completed());
        promises[i]->success(i * 2);
    }
    EXPECT_TRUE(sequencedFuture->completed());
    EXPECT_TRUE(sequencedFuture->succeeded());
    EXPECT_FALSE(sequencedFuture->failed());
    QList<int> result = sequencedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(FutureTest, sequenceQListNegative)
{
    int n = 5;
    QList<PromiseSP<int>> promises;
    for (int i = 0; i < n; ++i)
        promises << PromiseSP<int>::create();
    QList<FutureSP<int>> futures = algorithms::map(promises, [](auto p){return p->future();}, QList<FutureSP<int>>());
    FutureSP<QList<int>> sequencedFuture = Future<int>::sequence(futures);
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(futures[i]->completed());
    EXPECT_FALSE(sequencedFuture->completed());

    for (int i = 0; i < n - 2; ++i) {
        promises[i]->success(i * 2);
        EXPECT_FALSE(sequencedFuture->completed());
    }
    promises[n - 2]->failure("failed");
    EXPECT_TRUE(sequencedFuture->completed());
    EXPECT_FALSE(sequencedFuture->succeeded());
    EXPECT_TRUE(sequencedFuture->failed());
    EXPECT_EQ("failed", sequencedFuture->failureReason());
    EXPECT_TRUE(sequencedFuture->result().isEmpty());
}

TEST(FutureTest, sequenceQVector)
{
    int n = 5;
    QVector<PromiseSP<int>> promises;
    for (int i = 0; i < n; ++i)
        promises << PromiseSP<int>::create();
    QVector<FutureSP<int>> futures = algorithms::map(promises, [](auto p){return p->future();}, QVector<FutureSP<int>>());
    FutureSP<QVector<int>> sequencedFuture = Future<int>::sequence(futures);
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(futures[i]->completed());

    for (int i = 0; i < n; ++i) {
        EXPECT_FALSE(sequencedFuture->completed());
        promises[i]->success(i * 2);
    }
    EXPECT_TRUE(sequencedFuture->completed());
    EXPECT_TRUE(sequencedFuture->succeeded());
    EXPECT_FALSE(sequencedFuture->failed());
    QVector<int> result = sequencedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i * 2, result[i]);
}

TEST(FutureTest, sequenceQVectorNegative)
{
    int n = 5;
    QVector<PromiseSP<int>> promises;
    for (int i = 0; i < n; ++i)
        promises << PromiseSP<int>::create();
    QVector<FutureSP<int>> futures = algorithms::map(promises, [](auto p){return p->future();}, QVector<FutureSP<int>>());
    FutureSP<QVector<int>> sequencedFuture = Future<int>::sequence(futures);
    for (int i = 0; i < n; ++i)
        EXPECT_FALSE(futures[i]->completed());
    EXPECT_FALSE(sequencedFuture->completed());

    for (int i = 0; i < n - 2; ++i) {
        promises[i]->success(i * 2);
        EXPECT_FALSE(sequencedFuture->completed());
    }
    promises[n - 2]->failure("failed");
    EXPECT_TRUE(sequencedFuture->completed());
    EXPECT_FALSE(sequencedFuture->succeeded());
    EXPECT_TRUE(sequencedFuture->failed());
    EXPECT_EQ("failed", sequencedFuture->failureReason());
    EXPECT_TRUE(sequencedFuture->result().isEmpty());
}
