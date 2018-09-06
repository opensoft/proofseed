// clazy:skip

#include "proofseed/future.h"
#include "proofseed/proofalgorithms.h"

#include "gtest/test_global.h"

#include <QDateTime>
#include <QThread>

using namespace Proof;

TEST(FutureTest, onSuccessException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    int result = 0;
    promise->future()->onSuccess([](int) { throw std::runtime_error("Hi"); })->onSuccess([&result](int x) { result = x; });
    promise->success(42);
    EXPECT_EQ(42, result);
}

TEST(FutureTest, onFailureException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    Failure result;
    promise->future()->onFailure([](Failure) { throw std::runtime_error("Hi"); })->onFailure([&result](Failure x) {
        result = x;
    });
    promise->failure(Failure("failed", 0, 0));
    EXPECT_EQ("failed", result.message);
}

TEST(FutureTest, forEachException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    int result = 0;
    promise->future()->forEach([](int) { throw std::runtime_error("Hi"); })->forEach([&result](int x) { result = x; });
    promise->success(42);
    EXPECT_EQ(42, result);
}

TEST(FutureTest, mapException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->map([](int) -> int { throw std::runtime_error("Hi"); });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, mapExceptionNonStd)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->map([](int) -> int { throw 42; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, flatMapException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->flatMap([](int) -> FutureSP<int> { throw std::runtime_error("Hi"); });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, flatMapExceptionNonStd)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->flatMap([](int) -> FutureSP<int> { throw 42; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, andThenException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->andThen([]() -> FutureSP<int> { throw std::runtime_error("Hi"); });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, andThenExceptionNonStd)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->andThen([]() -> FutureSP<int> { throw 42; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, filterException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->filter([](int) -> bool { throw std::runtime_error("Hi"); });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, filterExceptionNonStd)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->filter([](int) -> bool { throw 42; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success(42);
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerReduceException)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerReduce([](int, int) -> int { throw std::runtime_error("Hi"); }, 0);
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerReduceExceptionNonStd)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerReduce([](int, int) -> int { throw 42; }, 0);
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerReduceByMutationException)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerReduceByMutation([](int, int) { throw std::runtime_error("Hi"); }, 0);
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerReduceByMutationExceptionNonStd)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerReduceByMutation([](int, int) { throw 42; }, 0);
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerMapException)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerMap([](int) -> int { throw std::runtime_error("Hi"); }, QVector<int>());
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerMapExceptionNonStd)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerMap([](int) -> int { throw 42; }, QVector<int>());
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerMapShortException)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerMap([](int) -> int { throw std::runtime_error("Hi"); });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, innerMapShortExceptionNonStd)
{
    PromiseSP<QVector<int>> promise = PromiseSP<QVector<int>>::create();
    auto mappedFuture = promise->future()->innerMap([](int) -> int { throw 42; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->success({42});
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, recoverException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->recover([](const Failure &) -> int { throw std::runtime_error("Hi"); });
    EXPECT_FALSE(mappedFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, recoverExceptionNonStd)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->recover([](const Failure &) -> int { throw 42; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}

TEST(FutureTest, recoverWithException)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->recoverWith(
        [](const Failure &) -> FutureSP<int> { throw std::runtime_error("Hi"); });
    EXPECT_FALSE(mappedFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught: Hi", mappedFuture->failureReason().message);
}

TEST(FutureTest, recoverWithExceptionNonStd)
{
    PromiseSP<int> promise = PromiseSP<int>::create();
    auto mappedFuture = promise->future()->recoverWith([](const Failure &) -> FutureSP<int> { throw 42; });
    EXPECT_FALSE(mappedFuture->completed());
    promise->failure(Failure("failed", 1, 2, Failure::UserFriendlyHint, 5));
    ASSERT_TRUE(mappedFuture->completed());
    EXPECT_FALSE(mappedFuture->succeeded());
    EXPECT_TRUE(mappedFuture->failed());
    EXPECT_EQ(Failure::FromExceptionHint, mappedFuture->failureReason().hints);
    EXPECT_EQ("Exception caught", mappedFuture->failureReason().message);
}
