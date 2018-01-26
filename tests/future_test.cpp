// clazy:skip

#include "gtest/test_global.h"

#include "proofcore/future.h"
#include "proofcore/proofalgorithms.h"

#include <QThread>
#include <QDateTime>

using namespace Proof;

TEST(FutureTest, success)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->success(42);
    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(future->failureReason().exists);
}

TEST(FutureTest, failure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);
    EXPECT_EQ(1, future->failureReason().moduleCode);
    EXPECT_EQ(2, future->failureReason().errorCode);
    EXPECT_EQ(QVariant(5), future->failureReason().data);
    EXPECT_EQ(Failure::UserFriendlyHint, future->failureReason().hints);
}

TEST(FutureTest, withFailure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->success(WithFailure<int>("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);
    EXPECT_EQ(1, future->failureReason().moduleCode);
    EXPECT_EQ(2, future->failureReason().errorCode);
    EXPECT_EQ(QVariant(5), future->failureReason().data);
    EXPECT_EQ(Failure::UserFriendlyHint, future->failureReason().hints);

    promise = PromiseSP<int>::create();
    future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->success(WithFailure<int>(Failure("failed2", 10, 20, Failure::UserFriendlyHint | Failure::CriticalHint, 50)));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed2", future->failureReason().message);
    EXPECT_EQ(10, future->failureReason().moduleCode);
    EXPECT_EQ(20, future->failureReason().errorCode);
    EXPECT_EQ(QVariant(50), future->failureReason().data);
    EXPECT_EQ(Failure::UserFriendlyHint | Failure::CriticalHint, future->failureReason().hints);

    promise = PromiseSP<int>::create();
    future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->success(WithFailure<int>("failed3", 100, 200));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed3", future->failureReason().message);
    EXPECT_EQ(100, future->failureReason().moduleCode);
    EXPECT_EQ(200, future->failureReason().errorCode);
    EXPECT_EQ(QVariant(), future->failureReason().data);
    EXPECT_EQ(Failure::NoHint, future->failureReason().hints);

    promise = PromiseSP<int>::create();
    future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->success(WithFailure<int>("failed4"));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ(QLatin1String(), future->failureReason().message);
    EXPECT_EQ(0, future->failureReason().moduleCode);
    EXPECT_EQ(0, future->failureReason().errorCode);
    EXPECT_EQ("failed4", future->failureReason().data);
    EXPECT_EQ(Failure::NoHint, future->failureReason().hints);
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
    Failure result;
    FutureSP<int> futureWithCallback = future->onFailure([&result](Failure x){result = x;});
    EXPECT_EQ(future, futureWithCallback);
    promise->failure(Failure("failed", 0, 0));
    EXPECT_EQ("failed", result.message);
}

TEST(FutureTest, multipleOnSuccess)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result[3] = {0, 0, 0};
    Failure failedResult[3] = {Failure(), Failure(), Failure()};
    for (int i = 0; i < 3; ++i) {
        FutureSP<int> futureWithCallback = future->onSuccess([&result = result[i]](int x){result = x;});
        EXPECT_EQ(future, futureWithCallback);
        futureWithCallback = future->onFailure([&failedResult = failedResult[i]](Failure x){failedResult = x;});
        EXPECT_EQ(future, futureWithCallback);
    }
    promise->success(42);
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(42, result[i]);
        EXPECT_FALSE(failedResult[i].exists);
    }
}

TEST(FutureTest, multipleOnFailure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result[3] = {0, 0, 0};
    Failure failedResult[3] = {Failure(), Failure(), Failure()};
    for (int i = 0; i < 3; ++i) {
        FutureSP<int> futureWithCallback = future->onSuccess([&result = result[i]](int x){result = x;});
        EXPECT_EQ(future, futureWithCallback);
        futureWithCallback = future->onFailure([&failedResult = failedResult[i]](Failure x){failedResult = x;});
        EXPECT_EQ(future, futureWithCallback);
    }
    promise->failure(Failure("failed", 0, 0));
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(0, result[i]);
        EXPECT_EQ("failed", failedResult[i].message);
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
    promise->failure(Failure("failed", 0, 0));
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
    ASSERT_TRUE(mappedFuture->completed());
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
    ASSERT_TRUE(mappedFuture->completed());
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
    ASSERT_TRUE(mappedFuture->completed());
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
    ASSERT_TRUE(mappedFuture->completed());
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
    ASSERT_TRUE(reducedFuture->completed());
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
    ASSERT_TRUE(sequencedFuture->completed());
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
    promises[n - 2]->failure(Failure("failed", 0, 0));
    ASSERT_TRUE(sequencedFuture->completed());
    EXPECT_FALSE(sequencedFuture->succeeded());
    EXPECT_TRUE(sequencedFuture->failed());
    EXPECT_EQ("failed", sequencedFuture->failureReason().message);
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
    ASSERT_TRUE(sequencedFuture->completed());
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
    promises[n - 2]->failure(Failure("failed", 0, 0));
    ASSERT_TRUE(sequencedFuture->completed());
    EXPECT_FALSE(sequencedFuture->succeeded());
    EXPECT_TRUE(sequencedFuture->failed());
    EXPECT_EQ("failed", sequencedFuture->failureReason().message);
    EXPECT_TRUE(sequencedFuture->result().isEmpty());
}

TEST(FutureTest, failureFromMap)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->map([](int x){return x * 2;});
    FutureSP<int> mappedAgainFuture = mappedFuture->map([](int) -> int {return WithFailure<int>("failed", 0, 0);});
    FutureSP<int> mappedOnceMoreFuture = mappedAgainFuture->map([](int) -> int {return 24;});
    EXPECT_FALSE(mappedFuture->completed());
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(84, mappedFuture->result());
    ASSERT_TRUE(mappedAgainFuture->completed());
    EXPECT_FALSE(mappedAgainFuture->succeeded());
    EXPECT_TRUE(mappedAgainFuture->failed());
    EXPECT_EQ("failed", mappedAgainFuture->failureReason().message);
    EXPECT_EQ(0, mappedAgainFuture->result());
    ASSERT_TRUE(mappedOnceMoreFuture->completed());
    EXPECT_FALSE(mappedOnceMoreFuture->succeeded());
    EXPECT_TRUE(mappedOnceMoreFuture->failed());
    EXPECT_EQ("failed", mappedOnceMoreFuture->failureReason().message);
    EXPECT_EQ(0, mappedOnceMoreFuture->result());
}

TEST(FutureTest, failureFromFlatMap)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->map([](int x){return x * 2;});
    FutureSP<int> mappedAgainFuture = mappedFuture->flatMap([](int) -> FutureSP<int> {return WithFailure<int>("failed", 0, 0);});
    FutureSP<int> mappedOnceMoreFuture = mappedAgainFuture->map([](int) -> int {return 24;});
    EXPECT_FALSE(mappedFuture->completed());
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(84, mappedFuture->result());
    ASSERT_TRUE(mappedAgainFuture->completed());
    EXPECT_FALSE(mappedAgainFuture->succeeded());
    EXPECT_TRUE(mappedAgainFuture->failed());
    EXPECT_EQ("failed", mappedAgainFuture->failureReason().message);
    EXPECT_EQ(0, mappedAgainFuture->result());
    ASSERT_TRUE(mappedOnceMoreFuture->completed());
    EXPECT_FALSE(mappedOnceMoreFuture->succeeded());
    EXPECT_TRUE(mappedOnceMoreFuture->failed());
    EXPECT_EQ("failed", mappedOnceMoreFuture->failureReason().message);
    EXPECT_EQ(0, mappedOnceMoreFuture->result());
}

TEST(FutureTest, failureFromReduce)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<QList<int>> mappedFuture = future->map([](int x){return QList<int>{x, x * 2};});
    FutureSP<int> mappedAgainFuture = mappedFuture->reduce([](int, int) -> int {return WithFailure<int>("failed", 0, 0);}, 0);
    FutureSP<int> mappedOnceMoreFuture = mappedAgainFuture->map([](int) -> int {return 24;});
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    ASSERT_EQ(2, mappedFuture->result().count());
    ASSERT_TRUE(mappedAgainFuture->completed());
    EXPECT_FALSE(mappedAgainFuture->succeeded());
    EXPECT_TRUE(mappedAgainFuture->failed());
    EXPECT_EQ("failed", mappedAgainFuture->failureReason().message);
    EXPECT_EQ(0, mappedAgainFuture->result());
    ASSERT_TRUE(mappedOnceMoreFuture->completed());
    EXPECT_FALSE(mappedOnceMoreFuture->succeeded());
    EXPECT_TRUE(mappedOnceMoreFuture->failed());
    EXPECT_EQ("failed", mappedOnceMoreFuture->failureReason().message);
    EXPECT_EQ(0, mappedOnceMoreFuture->result());
}

TEST(FutureTest, recover)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> recoveredFuture = future->recover([](const Failure &){return 42;});
    EXPECT_FALSE(future->completed());
    EXPECT_FALSE(recoveredFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);

    ASSERT_TRUE(recoveredFuture->completed());
    EXPECT_TRUE(recoveredFuture->succeeded());
    EXPECT_FALSE(recoveredFuture->failed());
    EXPECT_EQ(42, recoveredFuture->result());
}

TEST(FutureTest, recoverFromWithFailure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> recoveredFuture = future->recover([](const Failure &){return 42;});
    EXPECT_FALSE(future->completed());
    EXPECT_FALSE(recoveredFuture->completed());
    promise->success(WithFailure<int>("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);

    ASSERT_TRUE(recoveredFuture->completed());
    EXPECT_TRUE(recoveredFuture->succeeded());
    EXPECT_FALSE(recoveredFuture->failed());
    EXPECT_EQ(42, recoveredFuture->result());
}

TEST(FutureTest, recoverAndFail)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> recoveredFuture = future->recover([](const Failure &) -> int {return WithFailure<int>("failed2", 0, 0);});
    EXPECT_FALSE(future->completed());
    EXPECT_FALSE(recoveredFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);

    ASSERT_TRUE(recoveredFuture->completed());
    EXPECT_FALSE(recoveredFuture->succeeded());
    EXPECT_TRUE(recoveredFuture->failed());
    EXPECT_TRUE(recoveredFuture->failureReason().exists);
    EXPECT_EQ("failed2", recoveredFuture->failureReason().message);
    EXPECT_EQ(0, recoveredFuture->result());
}
