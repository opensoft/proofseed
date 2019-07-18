// clazy:skip

#include "proofseed/asynqro_extra.h"

#include "gtest/proof/test_global.h"

#include <QDateTime>
#include <QPair>
#include <QThread>
#include <QTimer>

using namespace Proof;
using namespace Proof::repeater;

using RepeatedDataResult = RepeaterResult<std::vector<int>, int, std::vector<int>>;

TEST(AsynqroExtraTest, failureWithMessage)
{
    const Failure failure("message", 10, 42, Failure::Hints::UserFriendlyHint, "data");
    Failure another = failure.withMessage("changed");
    EXPECT_EQ("changed", another.message);
    EXPECT_EQ(10, another.moduleCode);
    EXPECT_EQ(42, another.errorCode);
    EXPECT_EQ(Failure::Hints::UserFriendlyHint, another.hints);
    EXPECT_EQ(QVariant("data"), another.data);
}

TEST(AsynqroExtraTest, failureWithCode)
{
    const Failure failure("message", 10, 42, Failure::Hints::UserFriendlyHint, "data");
    Failure another = failure.withCode(11, 21);
    EXPECT_EQ("message", another.message);
    EXPECT_EQ(11, another.moduleCode);
    EXPECT_EQ(21, another.errorCode);
    EXPECT_EQ(Failure::Hints::UserFriendlyHint, another.hints);
    EXPECT_EQ(QVariant("data"), another.data);
}

TEST(AsynqroExtraTest, failureWithData)
{
    const Failure failure("message", 10, 42, Failure::Hints::UserFriendlyHint, "data");
    Failure another = failure.withData("other");
    EXPECT_EQ("message", another.message);
    EXPECT_EQ(10, another.moduleCode);
    EXPECT_EQ(42, another.errorCode);
    EXPECT_EQ(Failure::Hints::UserFriendlyHint, another.hints);
    EXPECT_EQ(QVariant("other"), another.data);
}

TEST(AsynqroExtraTest, failureFromString)
{
    Failure failure = asynqro::failure::failureFromString<Failure>("Message");
    EXPECT_EQ("Message", failure.message);
    EXPECT_EQ(0, failure.moduleCode);
    EXPECT_EQ(0, failure.errorCode);
    EXPECT_EQ(Failure::Hints::UserFriendlyHint, failure.hints);
    EXPECT_TRUE(failure.data.isNull());
}

TEST(AsynqroExtraTest, exceptionFailureFromString)
{
    Failure failure = asynqro::failure::failureFromString<Failure>("Exception caught");
    EXPECT_EQ("Exception caught", failure.message);
    EXPECT_EQ(0, failure.moduleCode);
    EXPECT_EQ(0, failure.errorCode);
    EXPECT_EQ(Failure::Hints::UserFriendlyHint | Failure::Hints::FromExceptionHint, failure.hints);
    EXPECT_TRUE(failure.data.isNull());
}

TEST(AsynqroExtraTest, futuresSuccessfulRValue)
{
    Future<int> f = futures::successful(42);
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    EXPECT_EQ(42, f.result());
}

TEST(AsynqroExtraTest, futuresSuccessfulLValue)
{
    int result = 42;
    Future<int> f = futures::successful(result);
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    EXPECT_EQ(42, f.result());
}

TEST(AsynqroExtraTest, futuresSequenceRValue)
{
    Future<std::vector<int>> f = futures::sequence(
        std::vector<Future<int>>{futures::successful(0), futures::successful(5), futures::successful(10),
                                 futures::successful(15), futures::successful(20)});
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    ASSERT_EQ(5, f.resultRef().size());
    for (unsigned i = 0; i < 5; ++i)
        EXPECT_EQ(5 * i, f.resultRef()[i]) << i;
}

TEST(AsynqroExtraTest, futuresSequenceLValue)
{
    std::vector<Future<int>> v{futures::successful(0), futures::successful(5), futures::successful(10),
                               futures::successful(15), futures::successful(20)};
    Future<std::vector<int>> f = futures::sequence(v);
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    ASSERT_EQ(5, f.resultRef().size());
    for (unsigned i = 0; i < 5; ++i)
        EXPECT_EQ(5 * i, f.resultRef()[i]) << i;
}

TEST(AsynqroExtraTest, futuresSequenceWithFailuresRValue)
{
    Future<std::pair<QHash<unsigned long, int>, QHash<unsigned long, Failure>>> f = futures::sequenceWithFailures(
        std::vector<Future<int>>{futures::successful(0), futures::successful(5), futures::successful(10),
                                 futures::successful(15), futures::successful(20)});
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    ASSERT_EQ(5, f.resultRef().first.size());
    for (unsigned i = 0; i < 5; ++i)
        EXPECT_EQ(5 * i, f.resultRef().first[i]) << i;
    ASSERT_EQ(0, f.resultRef().second.size());
}

TEST(AsynqroExtraTest, futuresSequenceWithFailuresLValue)
{
    std::vector<Future<int>> v{futures::successful(0), futures::successful(5), futures::successful(10),
                               futures::successful(15), futures::successful(20)};
    Future<std::pair<QHash<unsigned long, int>, QHash<unsigned long, Failure>>> f = futures::sequenceWithFailures(v);
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    ASSERT_EQ(5, f.resultRef().first.size());
    for (unsigned i = 0; i < 5; ++i)
        EXPECT_EQ(5 * i, f.resultRef().first[i]) << i;
    ASSERT_EQ(0, f.resultRef().second.size());
}

TEST(AsynqroExtraTest, futuresSequenceWithFailuresQMapRValue)
{
    Future<std::pair<QMap<unsigned long, int>, QMap<unsigned long, Failure>>> f = futures::sequenceWithFailures<QMap>(
        std::vector<Future<int>>{futures::successful(0), futures::successful(5), futures::successful(10),
                                 futures::successful(15), futures::successful(20)});
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    ASSERT_EQ(5, f.resultRef().first.size());
    for (unsigned i = 0; i < 5; ++i)
        EXPECT_EQ(5 * i, f.resultRef().first[i]) << i;
    ASSERT_EQ(0, f.resultRef().second.size());
}

TEST(AsynqroExtraTest, futuresSequenceWithFailuresQMapLValue)
{
    std::vector<Future<int>> v{futures::successful(0), futures::successful(5), futures::successful(10),
                               futures::successful(15), futures::successful(20)};
    Future<std::pair<QMap<unsigned long, int>, QMap<unsigned long, Failure>>> f = futures::sequenceWithFailures<QMap>(v);
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    ASSERT_EQ(5, f.resultRef().first.size());
    for (unsigned i = 0; i < 5; ++i)
        EXPECT_EQ(5 * i, f.resultRef().first[i]) << i;
    ASSERT_EQ(0, f.resultRef().second.size());
}

TEST(AsynqroExtraTest, repeatData)
{
    Future<std::vector<int>> f = futures::repeat<std::vector<int>>(
        [](int step, std::vector<int> order) -> RepeatedDataResult {
            order.push_back(step);
            if (step >= 99)
                return Finish(order);
            return Continue(step + 1, std::move(order));
        },
        0, std::vector<int>{});
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    EXPECT_EQ(100, f.resultRef().size());
    for (int i = 0; i < f.resultRef().size(); ++i)
        EXPECT_EQ(i, f.resultRef()[i]);
}

TEST(AsynqroExtraTest, repeatForSequenceRValue)
{
    std::vector<Promise<double>> promises(5);
    Future<std::vector<double>> f = futures::repeatForSequence(std::vector<size_t>{0, 1, 2, 3, 4}, std::vector<double>{},
                                                               [&promises](size_t x, std::vector<double> result) {
                                                                   return promises[x].future().map([result](double x) {
                                                                       auto newResult = result;
                                                                       newResult.push_back(x);
                                                                       return newResult;
                                                                   });
                                                               });
    promises[2].success(0.2);
    for (size_t i = 0; i < promises.size(); ++i) {
        ASSERT_FALSE(f.isCompleted());
        promises[i].success(i / 10.0);
    }
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    EXPECT_EQ(5, f.resultRef().size());
    for (size_t i = 0; i < f.resultRef().size(); ++i)
        EXPECT_DOUBLE_EQ(i / 10.0, f.resultRef()[i]);
}

TEST(AsynqroExtraTest, repeatForSequenceLValue)
{
    std::vector<Promise<double>> promises(5);
    std::vector<size_t> data = std::vector<size_t>{0, 1, 2, 3, 4};
    Future<std::vector<double>> f = futures::repeatForSequence(data, std::vector<double>{},
                                                               [&promises](size_t x, std::vector<double> result) {
                                                                   return promises[x].future().map([result](double x) {
                                                                       auto newResult = result;
                                                                       newResult.push_back(x);
                                                                       return newResult;
                                                                   });
                                                               });
    for (size_t i = 0; i < promises.size(); ++i) {
        ASSERT_FALSE(f.isCompleted());
        promises[i].success(i / 10.0);
    }
    ASSERT_TRUE(f.isCompleted());
    ASSERT_TRUE(f.isSucceeded());
    EXPECT_EQ(5, f.resultRef().size());
    for (size_t i = 0; i < f.resultRef().size(); ++i)
        EXPECT_DOUBLE_EQ(i / 10.0, f.resultRef()[i]);
}

TEST(AsynqroExtraTest, runWrapper)
{
    Future<std::thread::id> future = tasks::run([]() { return std::this_thread::get_id(); });
    future.wait(1000);
    ASSERT_TRUE(future.isCompleted());
    ASSERT_TRUE(future.isSucceeded());
    ASSERT_NE(std::this_thread::get_id(), future.result());
}

TEST(AsynqroExtraTest, runAndForgetWrapper)
{
    Promise<std::thread::id> promise;
    tasks::runAndForget([promise]() { promise.success(std::this_thread::get_id()); });
    auto future = promise.future();
    future.wait(1000);
    ASSERT_TRUE(future.isCompleted());
    ASSERT_TRUE(future.isSucceeded());
    ASSERT_NE(std::this_thread::get_id(), future.result());
}
