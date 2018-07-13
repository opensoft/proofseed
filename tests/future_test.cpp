// clazy:skip

#include "proofcore/future.h"
#include "proofcore/proofalgorithms.h"

#include <QDateTime>
#include <QThread>

#include "gtest/test_global.h"

using namespace Proof;

TEST(FutureTest, successful)
{
    FutureSP<int> future = Future<int>::successful(42);
    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(future->failureReason().exists);
}

TEST(FutureTest, successfulEmpty)
{
    FutureSP<int> future = Future<int>::successful();
    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(0, future->result());
    EXPECT_FALSE(future->failureReason().exists);
}

TEST(FutureTest, successfulNoT)
{
    FutureSP<int> future = Future<>::successful(42);
    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(future->failureReason().exists);
}

TEST(FutureTest, fail)
{
    FutureSP<int> future = Future<int>::fail(Failure("failed", 0, 0));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);
}

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

TEST(FutureTest, cancelation)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    CancelableFuture<int> future(promise);
    EXPECT_FALSE(future->completed());
    future.cancel();
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("Canceled", future->failureReason().message);
    promise->success(42);
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("Canceled", future->failureReason().message);
}

TEST(FutureTest, withFailure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    EXPECT_FALSE(future->completed());
    promise->success(WithFailure("failed", 1, 2, Failure::UserFriendlyHint, 5));
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
    promise->success(WithFailure(Failure("failed2", 10, 20, Failure::UserFriendlyHint | Failure::CriticalHint, 50)));
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
    promise->success(WithFailure("failed3", 100, 200));
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
    promise->success(WithFailure("failed4"));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ(QString(), future->failureReason().message);
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
    FutureSP<int> futureWithCallback = future->onSuccess([&result](int x) { result = x; });
    EXPECT_EQ(future, futureWithCallback);
    promise->success(42);
    EXPECT_EQ(42, result);
}

TEST(FutureTest, onFailure)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    Failure result;
    FutureSP<int> futureWithCallback = future->onFailure([&result](Failure x) { result = x; });
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
        FutureSP<int> futureWithCallback = future->onSuccess([&result = result[i]](int x) { result = x; });
        EXPECT_EQ(future, futureWithCallback);
        futureWithCallback = future->onFailure([&failedResult = failedResult[i]](Failure x) { failedResult = x; });
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
        FutureSP<int> futureWithCallback = future->onSuccess([&result = result[i]](int x) { result = x; });
        EXPECT_EQ(future, futureWithCallback);
        futureWithCallback = future->onFailure([&failedResult = failedResult[i]](Failure x) { failedResult = x; });
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
    FutureSP<int> futureWithCallback = future->forEach([&result](int x) { result = x; });
    EXPECT_EQ(future, futureWithCallback);
    promise->success(42);
    EXPECT_EQ(42, result);
}

TEST(FutureTest, forEachNegative)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result = 0;
    FutureSP<int> futureWithCallback = future->forEach([&result](int x) { result = x; });
    EXPECT_EQ(future, futureWithCallback);
    promise->failure(Failure("failed", 0, 0));
    EXPECT_EQ(0, result);
}

TEST(FutureTest, map)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->map([](int x) { return x * 2; });
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
    FutureSP<int> mappedFuture = future->flatMap(
        [innerPromise](int x) { return innerPromise->future()->map([x](int y) { return x * y; }); });
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

TEST(FutureTest, andThen)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    PromiseSP<int> innerPromise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->andThen([innerPromise]() { return innerPromise->future(); });
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(mappedFuture->completed());
    innerPromise->success(2);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(2, mappedFuture->result());
}

TEST(FutureTest, andThenValueR)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->andThenValue(2);
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(2, mappedFuture->result());
}

TEST(FutureTest, andThenValueL)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    int result = 2;
    FutureSP<int> mappedFuture = future->andThenValue(result);
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(2, mappedFuture->result());
    EXPECT_EQ(2, result);
}

TEST(FutureTest, andThenValueCL)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    const int result = 2;
    FutureSP<int> mappedFuture = future->andThenValue(result);
    EXPECT_NE(future, mappedFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_EQ(2, mappedFuture->result());
}

TEST(FutureTest, differentTypeMap)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<double> mappedFuture = future->map([](int x) -> double { return x / 2.0; });
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
    FutureSP<double> mappedFuture = future->flatMap(
        [innerPromise](int x) { return innerPromise->future()->map([x](double y) { return x / y; }); });
    promise->success(42);
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(mappedFuture->completed());
    innerPromise->success(2.0);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_DOUBLE_EQ(21.0, mappedFuture->result());
}

TEST(FutureTest, differentTypeAndThen)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    PromiseSP<double> innerPromise = PromiseSP<double>::create();
    FutureSP<int> future = promise->future();
    FutureSP<double> mappedFuture = future->andThen([innerPromise]() { return innerPromise->future(); });
    promise->success(42);
    EXPECT_EQ(42, future->result());
    EXPECT_FALSE(mappedFuture->completed());
    innerPromise->success(2.0);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_DOUBLE_EQ(2.0, mappedFuture->result());
}

TEST(FutureTest, differentTypeAndThenValueR)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<double> mappedFuture = future->andThenValue(2.0);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_DOUBLE_EQ(2.0, mappedFuture->result());
}

TEST(FutureTest, differentTypeAndThenValueL)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    double result = 2.0;
    FutureSP<double> mappedFuture = future->andThenValue(result);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_DOUBLE_EQ(2.0, mappedFuture->result());
    EXPECT_DOUBLE_EQ(2.0, result);
}

TEST(FutureTest, differentTypeAndThenValueCL)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    const double result = 2.0;
    FutureSP<double> mappedFuture = future->andThenValue(result);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    EXPECT_DOUBLE_EQ(2.0, mappedFuture->result());
}

TEST(FutureTest, filterPositive)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> filteredFuture = future->filter([](int x) -> bool { return x % 2; });
    EXPECT_FALSE(filteredFuture->completed());
    EXPECT_NE(future, filteredFuture);
    promise->success(41);
    EXPECT_EQ(41, future->result());
    ASSERT_TRUE(filteredFuture->completed());
    EXPECT_TRUE(filteredFuture->succeeded());
    EXPECT_FALSE(filteredFuture->failed());
    EXPECT_EQ(41, filteredFuture->result());
}

TEST(FutureTest, filterNegative)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> filteredFuture = future->filter([](int x) { return x % 2; });
    EXPECT_FALSE(filteredFuture->completed());
    EXPECT_NE(future, filteredFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(filteredFuture->completed());
    EXPECT_FALSE(filteredFuture->succeeded());
    EXPECT_TRUE(filteredFuture->failed());
    EXPECT_EQ("Result wasn't good enough", filteredFuture->failureReason().message);
}

TEST(FutureTest, filterNegativeCustomRejectedR)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> filteredFuture = future->filter([](int) { return false; }, Failure("Custom", 0, 0));
    EXPECT_FALSE(filteredFuture->completed());
    EXPECT_NE(future, filteredFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(filteredFuture->completed());
    EXPECT_FALSE(filteredFuture->succeeded());
    EXPECT_TRUE(filteredFuture->failed());
    EXPECT_EQ("Custom", filteredFuture->failureReason().message);
}

TEST(FutureTest, filterNegativeCustomRejectedL)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    Failure rejected = Failure("Custom", 0, 0);
    FutureSP<int> filteredFuture = future->filter([](int) { return false; }, rejected);
    EXPECT_FALSE(filteredFuture->completed());
    EXPECT_NE(future, filteredFuture);
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(filteredFuture->completed());
    EXPECT_FALSE(filteredFuture->succeeded());
    EXPECT_TRUE(filteredFuture->failed());
    EXPECT_EQ("Custom", filteredFuture->failureReason().message);
}

TEST(FutureTest, innerReduce)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<int> reducedFuture = future->innerReduce([](int acc, int x) { return acc + x; }, 0);
    EXPECT_FALSE(reducedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    QVector<int> result = future->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i + 1, result[i]);
    ASSERT_TRUE(reducedFuture->completed());
    EXPECT_TRUE(reducedFuture->succeeded());
    EXPECT_FALSE(reducedFuture->failed());
    EXPECT_EQ(15, reducedFuture->result());
}

TEST(FutureTest, innerReduceByMutation)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<int> reducedFuture = future->innerReduceByMutation([](int &acc, int x) { acc += x; }, 0);
    EXPECT_FALSE(reducedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    QVector<int> result = future->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i + 1, result[i]);
    ASSERT_TRUE(reducedFuture->completed());
    EXPECT_TRUE(reducedFuture->succeeded());
    EXPECT_FALSE(reducedFuture->failed());
    EXPECT_EQ(15, reducedFuture->result());
}

TEST(FutureTest, innerMap)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QVector<int>> mappedFuture = future->innerMap([](int x) { return x + 10; }, QVector<int>());
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QVector<int> result = mappedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i + 11, result[i]);
}

TEST(FutureTest, innerMapShort)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QVector<int>> mappedFuture = future->innerMap([](int x) { return x + 10; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QVector<int> result = mappedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i + 11, result[i]);
}

TEST(FutureTest, innerMapOtherType)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QVector<double>> mappedFuture = future->innerMap([](int x) -> double { return x + 10.0; },
                                                              QVector<double>());
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QVector<double> result = mappedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_DOUBLE_EQ(i + 11, result[i]);
}

TEST(FutureTest, innerMapOtherTypeShort)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QVector<double>> mappedFuture = future->innerMap([](int x) -> double { return x + 10.0; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QVector<double> result = mappedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_DOUBLE_EQ(i + 11, result[i]);
}

TEST(FutureTest, innerMapOtherContainer)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QMap<int, bool>> mappedFuture = future->innerMap([](int x) { return qMakePair(x, x % 2); },
                                                              QMap<int, bool>());
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QMap<int, bool> result = mappedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 1; i <= 5; ++i) {
        EXPECT_TRUE(result.contains(i));
        EXPECT_EQ(i % 2, result[i]);
    }
}

TEST(FutureTest, innerMapWithIndex)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QVector<int>> mappedFuture = future->innerMap([](long long index, int x) { return x + index * 10; },
                                                           QVector<int>());
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QVector<int> result = mappedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i * 10 + (i + 1), result[i]);
}

TEST(FutureTest, innerMapWithIndexShort)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QVector<long long>> mappedFuture = future->innerMap([](long long index, int x) { return x + index * 10; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QVector<long long> result = mappedFuture->result();
    ASSERT_EQ(5, result.count());
    for (int i = 0; i < 5; ++i)
        EXPECT_EQ(i * 10 + (i + 1), result[i]);
}

TEST(FutureTest, innerFilter)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    FutureSP<QVector<int>> future = promise->future();
    FutureSP<QVector<int>> mappedFuture = future->innerFilter([](int x) { return x % 2; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({1, 2, 3, 4, 5});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    QVector<int> result = mappedFuture->result();
    ASSERT_EQ(3, result.count());
    for (int i = 0; i < 3; ++i)
        EXPECT_EQ(i * 2 + 1, result[i]);
}

TEST(FutureTest, sequenceQVector)
{
    int n = 5;
    QVector<PromiseSP<int>> promises;
    for (int i = 0; i < n; ++i)
        promises << PromiseSP<int>::create();
    QVector<FutureSP<int>> futures = algorithms::map(promises, [](auto p) { return p->future(); });
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

TEST(FutureTest, sequenceQVectorNoT)
{
    int n = 5;
    QVector<PromiseSP<int>> promises;
    for (int i = 0; i < n; ++i)
        promises << PromiseSP<int>::create();
    QVector<FutureSP<int>> futures = algorithms::map(promises, [](auto p) { return p->future(); });
    FutureSP<QVector<int>> sequencedFuture = Future<>::sequence(futures);
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
    QVector<FutureSP<int>> futures = algorithms::map(promises, [](auto p) { return p->future(); });
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

TEST(FutureTest, sequenceQVectorEmpty)
{
    QVector<FutureSP<int>> futures;
    FutureSP<QVector<int>> sequencedFuture = Future<int>::sequence(futures);
    ASSERT_TRUE(sequencedFuture->completed());
    EXPECT_TRUE(sequencedFuture->succeeded());
    EXPECT_FALSE(sequencedFuture->failed());
    QVector<int> result = sequencedFuture->result();
    EXPECT_EQ(0, result.count());
}

TEST(FutureTest, failureFromMap)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->map([](int x) { return x * 2; });
    FutureSP<int> mappedAgainFuture = mappedFuture->map([](int) -> int { return WithFailure("failed", 0, 0); });
    FutureSP<int> mappedOnceMoreFuture = mappedAgainFuture->map([](int) -> int { return 24; });
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
    FutureSP<int> mappedFuture = future->map([](int x) { return x * 2; });
    FutureSP<int> mappedAgainFuture = mappedFuture->flatMap(
        [](int) -> FutureSP<int> { return WithFailure("failed", 0, 0); });
    FutureSP<int> mappedOnceMoreFuture = mappedAgainFuture->map([](int) -> int { return 24; });
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

TEST(FutureTest, failureFromAndThen)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> mappedFuture = future->map([](int x) { return x * 2; });
    FutureSP<int> mappedAgainFuture = mappedFuture->andThen([]() -> FutureSP<int> { return WithFailure("failed", 0, 0); });
    FutureSP<int> mappedOnceMoreFuture = mappedAgainFuture->map([](int) -> int { return 24; });
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

TEST(FutureTest, failureFromInnerReduce)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<QVector<int>> mappedFuture = future->map([](int x) { return QVector<int>{x, x * 2}; });
    auto reducedFuture = mappedFuture->innerReduce([](int, int) -> int { return WithFailure("failed", 0, 0); }, 0);
    FutureSP<int> mappedOnceMoreFuture = reducedFuture->map([](int) -> int { return 24; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    EXPECT_EQ(42, future->result());
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_TRUE(mappedFuture->succeeded());
    EXPECT_FALSE(mappedFuture->failed());
    ASSERT_EQ(2, mappedFuture->result().count());
    ASSERT_TRUE(reducedFuture->completed());
    EXPECT_FALSE(reducedFuture->succeeded());
    EXPECT_TRUE(reducedFuture->failed());
    EXPECT_EQ("failed", reducedFuture->failureReason().message);
    EXPECT_EQ(0, reducedFuture->result());
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
    FutureSP<int> recoveredFuture = future->recover([](const Failure &) { return 42; });
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
    FutureSP<int> recoveredFuture = future->recover([](const Failure &) { return 42; });
    EXPECT_NE(future, recoveredFuture);
    EXPECT_FALSE(future->completed());
    EXPECT_FALSE(recoveredFuture->completed());
    promise->success(WithFailure("failed", 1, 2, Failure::UserFriendlyHint, 5));
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
    FutureSP<int> recoveredFuture = future->recover([](const Failure &) -> int { return WithFailure("failed2", 0, 0); });
    EXPECT_NE(future, recoveredFuture);
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

TEST(FutureTest, recoverWith)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    PromiseSP<int> innerPromise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> recoveredFuture = future->recoverWith(
        [innerPromise](const Failure &) { return innerPromise->future(); });
    EXPECT_NE(future, recoveredFuture);
    EXPECT_FALSE(future->completed());
    EXPECT_FALSE(recoveredFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);
    EXPECT_FALSE(recoveredFuture->completed());

    innerPromise->success(42);
    ASSERT_TRUE(recoveredFuture->completed());
    EXPECT_TRUE(recoveredFuture->succeeded());
    EXPECT_FALSE(recoveredFuture->failed());
    EXPECT_EQ(42, recoveredFuture->result());
}

TEST(FutureTest, recoverWithAndFail)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    PromiseSP<int> innerPromise = PromiseSP<int>::create();
    FutureSP<int> future = promise->future();
    FutureSP<int> recoveredFuture = future->recoverWith(
        [innerPromise](const Failure &) { return innerPromise->future(); });
    EXPECT_NE(future, recoveredFuture);
    EXPECT_FALSE(future->completed());
    EXPECT_FALSE(recoveredFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_TRUE(future->failureReason().exists);
    EXPECT_EQ("failed", future->failureReason().message);
    EXPECT_FALSE(recoveredFuture->completed());

    innerPromise->failure(Failure("failed2", 0, 0));
    ASSERT_TRUE(recoveredFuture->completed());
    EXPECT_FALSE(recoveredFuture->succeeded());
    EXPECT_TRUE(recoveredFuture->failureReason().exists);
    EXPECT_EQ("failed2", recoveredFuture->failureReason().message);
}

TEST(FutureTest, zip)
{
    PromiseSP<int> firstPromise = PromiseSP<int>::create();
    PromiseSP<double> secondPromise = PromiseSP<double>::create();
    PromiseSP<QString> thirdPromise = PromiseSP<QString>::create();
    FutureSP<std::tuple<int, double, QString>> future = firstPromise->future()->zip(secondPromise->future(),
                                                                                    thirdPromise->future());
    EXPECT_FALSE(future->succeeded());
    secondPromise->success(5.0);
    EXPECT_FALSE(future->succeeded());
    firstPromise->success(42);
    EXPECT_FALSE(future->succeeded());
    thirdPromise->success("Done");

    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, std::get<0>(future->result()));
    EXPECT_DOUBLE_EQ(5.0, std::get<1>(future->result()));
    EXPECT_EQ("Done", std::get<2>(future->result()));
}

TEST(FutureTest, zipLeftTuple)
{
    PromiseSP<std::tuple<int, double>> firstPromise = PromiseSP<std::tuple<int, double>>::create();
    PromiseSP<QString> secondPromise = PromiseSP<QString>::create();
    FutureSP<std::tuple<int, double, QString>> future = firstPromise->future()->zip(secondPromise->future());
    EXPECT_FALSE(future->succeeded());
    secondPromise->success("Done");
    EXPECT_FALSE(future->succeeded());
    firstPromise->success(std::make_tuple(42, 5.0));

    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, std::get<0>(future->result()));
    EXPECT_DOUBLE_EQ(5.0, std::get<1>(future->result()));
    EXPECT_EQ("Done", std::get<2>(future->result()));
}

TEST(FutureTest, zipRightTuple)
{
    PromiseSP<std::tuple<int, double>> firstPromise = PromiseSP<std::tuple<int, double>>::create();
    PromiseSP<QString> secondPromise = PromiseSP<QString>::create();
    FutureSP<std::tuple<QString, int, double>> future = secondPromise->future()->zip(firstPromise->future());
    EXPECT_FALSE(future->succeeded());
    secondPromise->success("Done");
    EXPECT_FALSE(future->succeeded());
    firstPromise->success(std::make_tuple(42, 5.0));

    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ("Done", std::get<0>(future->result()));
    EXPECT_EQ(42, std::get<1>(future->result()));
    EXPECT_DOUBLE_EQ(5.0, std::get<2>(future->result()));
}

TEST(FutureTest, zipBothTuples)
{
    PromiseSP<std::tuple<int, double>> firstPromise = PromiseSP<std::tuple<int, double>>::create();
    PromiseSP<std::tuple<QString, int>> secondPromise = PromiseSP<std::tuple<QString, int>>::create();
    FutureSP<std::tuple<int, double, QString, int>> future = firstPromise->future()->zip(secondPromise->future());
    EXPECT_FALSE(future->succeeded());
    secondPromise->success(std::make_tuple("Done", 1024));
    EXPECT_FALSE(future->succeeded());
    firstPromise->success(std::make_tuple(42, 5.0));

    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, std::get<0>(future->result()));
    EXPECT_DOUBLE_EQ(5.0, std::get<1>(future->result()));
    EXPECT_EQ("Done", std::get<2>(future->result()));
    EXPECT_EQ(1024, std::get<3>(future->result()));
}

TEST(FutureTest, zipLeftFails)
{
    PromiseSP<int> firstPromise = PromiseSP<int>::create();
    PromiseSP<double> secondPromise = PromiseSP<double>::create();
    FutureSP<std::tuple<int, double>> future = firstPromise->future()->zip(secondPromise->future());
    EXPECT_FALSE(future->succeeded());
    secondPromise->success(5.0);
    EXPECT_FALSE(future->succeeded());
    firstPromise->failure(Failure("failed", 0, 0));

    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_EQ("failed", future->failureReason().message);
}

TEST(FutureTest, zipRightFails)
{
    PromiseSP<int> firstPromise = PromiseSP<int>::create();
    PromiseSP<double> secondPromise = PromiseSP<double>::create();
    FutureSP<std::tuple<int, double>> future = firstPromise->future()->zip(secondPromise->future());
    EXPECT_FALSE(future->succeeded());
    firstPromise->success(42);
    EXPECT_FALSE(future->succeeded());
    secondPromise->failure(Failure("failed", 0, 0));

    ASSERT_TRUE(future->completed());
    EXPECT_FALSE(future->succeeded());
    EXPECT_TRUE(future->failed());
    EXPECT_EQ("failed", future->failureReason().message);
}

TEST(FutureTest, zipValue)
{
    PromiseSP<double> firstPromise = PromiseSP<double>::create();
    FutureSP<std::tuple<double, int>> future = firstPromise->future()->zipValue(42);
    EXPECT_FALSE(future->succeeded());
    firstPromise->success(5.0);

    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_DOUBLE_EQ(5.0, std::get<0>(future->result()));
    EXPECT_EQ(42, std::get<1>(future->result()));
}

TEST(FutureTest, zipValueLeftTuple)
{
    PromiseSP<std::tuple<int, double>> firstPromise = PromiseSP<std::tuple<int, double>>::create();
    FutureSP<std::tuple<int, double, QString>> future = firstPromise->future()->zipValue(QStringLiteral("Done"));
    EXPECT_FALSE(future->succeeded());
    firstPromise->success(std::make_tuple(42, 5.0));

    ASSERT_TRUE(future->completed());
    EXPECT_TRUE(future->succeeded());
    EXPECT_FALSE(future->failed());
    EXPECT_EQ(42, std::get<0>(future->result()));
    EXPECT_DOUBLE_EQ(5.0, std::get<1>(future->result()));
    EXPECT_EQ("Done", std::get<2>(future->result()));
}
