// clazy:skip

#include "gtest/test_global.h"

#include "proofcore/proofalgorithms.h"

#include <QList>
#include <QSet>
#include <set>
#include <vector>
#include <QMap>

using namespace Proof;

template <typename T>
std::vector<T> setToVector(const std::set<T> &x)
{
    std::vector<T> result;
    std::copy(x.begin(), x.end(), std::back_inserter(result));
    return result;
}

TEST(AlgorithmsTest, eraseIfQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> result;

    auto evenPredicate = [](int x)->bool{return !(x % 2);};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto smallValuePredicate = [](int x)->bool{return x < 42;};
    auto nonEqualPredicate = [](int x)->bool{return x != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, evenPredicate);
    EXPECT_EQ(0, result.size());
    result = testContainer;
    algorithms::eraseIf(result, evenPredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9, result.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, result[i]);

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, eraseIfVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> result;

    auto evenPredicate = [](int x)->bool{return !(x % 2);};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto smallValuePredicate = [](int x)->bool{return x < 42;};
    auto nonEqualPredicate = [](int x)->bool{return x != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, evenPredicate);
    EXPECT_EQ(0u, result.size());
    result = testContainer;
    algorithms::eraseIf(result, evenPredicate);
    ASSERT_EQ(5u, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9u, result.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, result[i]);

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0u, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, eraseIfQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QMap<int, bool> result;

    auto falseValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(key); return !value;};
    auto bigValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key > 42;};
    auto smallValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key < 42;};
    auto nonEqualPredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    EXPECT_EQ(0, result.size());
    result = testContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_TRUE(result.contains(1));
    EXPECT_TRUE(result.contains(3));
    EXPECT_TRUE(result.contains(5));
    EXPECT_TRUE(result.contains(7));
    EXPECT_TRUE(result.contains(9));

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9, result.size());

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_TRUE(result.contains(5));
}

TEST(AlgorithmsTest, eraseIfQHash)
{
    QHash<int, bool> emptyContainer;
    QHash<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QHash<int, bool> result;

    auto falseValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(key); return !value;};
    auto bigValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key > 42;};
    auto smallValuePredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key < 42;};
    auto nonEqualPredicate = [](const auto &key, const auto &value)->bool{Q_UNUSED(value); return key != 5;};

    result = emptyContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    EXPECT_EQ(0, result.size());
    result = testContainer;
    algorithms::eraseIf(result, falseValuePredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_TRUE(result.contains(1));
    EXPECT_TRUE(result.contains(3));
    EXPECT_TRUE(result.contains(5));
    EXPECT_TRUE(result.contains(7));
    EXPECT_TRUE(result.contains(9));

    result = testContainer;
    algorithms::eraseIf(result, bigValuePredicate);
    ASSERT_EQ(9, result.size());

    result = testContainer;
    algorithms::eraseIf(result, smallValuePredicate);
    ASSERT_EQ(0, result.size());

    result = testContainer;
    algorithms::eraseIf(result, nonEqualPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_TRUE(result.contains(5));
}

TEST(AlgorithmsTest, makeUniqueQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 1, 2, 1, 3, 3, 3, 4, 1};
    QList<int> result;

    result = emptyContainer;
    algorithms::makeUnique(result);
    EXPECT_EQ(0, result.size());
    result = testContainer;
    algorithms::makeUnique(result);
    ASSERT_EQ(6, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(2, result[1]);
    EXPECT_EQ(1, result[2]);
    EXPECT_EQ(3, result[3]);
    EXPECT_EQ(4, result[4]);
    EXPECT_EQ(1, result[5]);
}

TEST(AlgorithmsTest, makeUniqueVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 1, 2, 1, 3, 3, 3, 4, 1};
    std::vector<int> result;

    result = emptyContainer;
    algorithms::makeUnique(result);
    EXPECT_EQ(0u, result.size());
    result = testContainer;
    algorithms::makeUnique(result);
    ASSERT_EQ(6u, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(2, result[1]);
    EXPECT_EQ(1, result[2]);
    EXPECT_EQ(3, result[3]);
    EXPECT_EQ(4, result[4]);
    EXPECT_EQ(1, result[5]);
}

TEST(AlgorithmsTest, findIfQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_EQ(1, result);

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, findIfQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_TRUE((QSet<int>{1, 3, 5, 7, 9}).contains(result));

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, findIfQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QPair<int, bool> result;

    auto oddPredicate = [](int, bool y)->bool{return y;};
    auto bigValuePredicate = [](int x, bool)->bool{return x > 42;};
    auto equalPredicate = [](int x, bool)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, qMakePair(-1, false));
    EXPECT_EQ(-1, result.first);
    EXPECT_FALSE(result.second);
    result = algorithms::findIf(testContainer, oddPredicate, qMakePair(-1, false));
    ASSERT_EQ(1, result.first);
    ASSERT_TRUE(result.second);

    result = algorithms::findIf(testContainer, bigValuePredicate, qMakePair(-1, false));
    EXPECT_EQ(-1, result.first);
    EXPECT_FALSE(result.second);
    result = algorithms::findIf(testContainer, bigValuePredicate, qMakePair(50, true));
    EXPECT_EQ(50, result.first);
    EXPECT_TRUE(result.second);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    EXPECT_EQ(0, result.first);
    EXPECT_FALSE(result.second);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result.first);
    EXPECT_TRUE(result.second);
}

TEST(AlgorithmsTest, findIfVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_EQ(1, result);

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, findIfSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    int result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::findIf(emptyContainer, oddPredicate, -1);
    EXPECT_EQ(-1, result);
    result = algorithms::findIf(testContainer, oddPredicate, -1);
    ASSERT_EQ(1, result);

    result = algorithms::findIf(testContainer, bigValuePredicate, -1);
    ASSERT_EQ(-1, result);
    result = algorithms::findIf(testContainer, bigValuePredicate, 50);
    ASSERT_EQ(50, result);
    result = algorithms::findIf(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result);

    result = algorithms::findIf(testContainer, equalPredicate);
    EXPECT_EQ(5, result);
}

TEST(AlgorithmsTest, filterQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate);
    EXPECT_EQ(0, result.size());
    result = algorithms::filter(testContainer, oddPredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = algorithms::filter(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result.size());

    result = algorithms::filter(testContainer, equalPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, filterQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate).toList();
    EXPECT_EQ(0, result.size());
    result = algorithms::filter(testContainer, oddPredicate).toList();
    ASSERT_EQ(5, result.size());
    std::sort(result.begin(), result.end());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = algorithms::filter(testContainer, bigValuePredicate).toList();
    ASSERT_EQ(0, result.size());

    result = algorithms::filter(testContainer, equalPredicate).toList();
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, filterQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QMap<int, bool> result;

    auto oddPredicate = [](int, bool y)->bool{return y;};
    auto bigValuePredicate = [](int x, bool)->bool{return x > 42;};
    auto equalPredicate = [](int x, bool)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate);
    EXPECT_EQ(0, result.size());
    result = algorithms::filter(testContainer, oddPredicate);
    ASSERT_EQ(5, result.size());
    EXPECT_TRUE(result.contains(1));
    EXPECT_TRUE(result.contains(3));
    EXPECT_TRUE(result.contains(5));
    EXPECT_TRUE(result.contains(7));
    EXPECT_TRUE(result.contains(9));

    result = algorithms::filter(testContainer, bigValuePredicate);
    ASSERT_EQ(0, result.size());

    result = algorithms::filter(testContainer, equalPredicate);
    ASSERT_EQ(1, result.size());
    EXPECT_TRUE(result.contains(5));
}

TEST(AlgorithmsTest, filterVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = algorithms::filter(emptyContainer, oddPredicate);
    EXPECT_EQ(0u, result.size());
    result = algorithms::filter(testContainer, oddPredicate);
    ASSERT_EQ(5u, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = algorithms::filter(testContainer, bigValuePredicate);
    ASSERT_EQ(0u, result.size());

    result = algorithms::filter(testContainer, equalPredicate);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, filterSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> result;

    auto oddPredicate = [](int x)->bool{return x % 2;};
    auto bigValuePredicate = [](int x)->bool{return x > 42;};
    auto equalPredicate = [](int x)->bool{return x == 5;};

    result = setToVector(algorithms::filter(emptyContainer, oddPredicate));
    EXPECT_EQ(0u, result.size());
    result = setToVector(algorithms::filter(testContainer, oddPredicate));
    ASSERT_EQ(5u, result.size());
    EXPECT_EQ(1, result[0]);
    EXPECT_EQ(3, result[1]);
    EXPECT_EQ(5, result[2]);
    EXPECT_EQ(7, result[3]);
    EXPECT_EQ(9, result[4]);

    result = setToVector(algorithms::filter(testContainer, bigValuePredicate));
    ASSERT_EQ(0u, result.size());

    result = setToVector(algorithms::filter(testContainer, equalPredicate));
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(5, result[0]);
}

TEST(AlgorithmsTest, reduceQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    struct Result {
        int sum;
        int mult;
    };
    int result;
    Result complexResult;
    int sum = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
    int mult = 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9;

    auto simpleReducer = [](int acc, int x)->int{return acc * x;};
    auto complexReducer = [](Result acc, int x)->Result{return Result{acc.sum + x, acc.mult * x};};

    result = algorithms::reduce(testContainer, simpleReducer, 1);
    EXPECT_EQ(mult, result);
    complexResult = algorithms::reduce(testContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(sum, complexResult.sum);
    EXPECT_EQ(mult, complexResult.mult);
    complexResult = algorithms::reduce(emptyContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(0, complexResult.sum);
    EXPECT_EQ(1, complexResult.mult);
}

TEST(AlgorithmsTest, reduceQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    struct Result {
        int sum;
        int mult;
    };
    int result;
    Result complexResult;
    int sum = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
    int mult = 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9;

    auto simpleReducer = [](int acc, int x)->int{return acc * x;};
    auto complexReducer = [](Result acc, int x)->Result{return Result{acc.sum + x, acc.mult * x};};

    result = algorithms::reduce(testContainer, simpleReducer, 1);
    EXPECT_EQ(mult, result);
    complexResult = algorithms::reduce(testContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(sum, complexResult.sum);
    EXPECT_EQ(mult, complexResult.mult);
    complexResult = algorithms::reduce(emptyContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(0, complexResult.sum);
    EXPECT_EQ(1, complexResult.mult);
}

TEST(AlgorithmsTest, reduceQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    struct Result {
        int sum;
        int mult;
    };
    int result;
    Result complexResult;
    int sum = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
    int mult = 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9;
    int simpleMult = 1 * 3 * 5 * 7 * 9;

    auto simpleReducer = [](int acc, int x, bool y)->int{return y ? acc * x : acc;};
    auto complexReducer = [](Result acc, int x, bool)->Result{return Result{acc.sum + x, acc.mult * x};};

    result = algorithms::reduce(testContainer, simpleReducer, 1);
    EXPECT_EQ(simpleMult, result);
    complexResult = algorithms::reduce(testContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(sum, complexResult.sum);
    EXPECT_EQ(mult, complexResult.mult);
    complexResult = algorithms::reduce(emptyContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(0, complexResult.sum);
    EXPECT_EQ(1, complexResult.mult);
}

TEST(AlgorithmsTest, reduceVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    struct Result {
        int sum;
        int mult;
    };
    int result;
    Result complexResult;
    int sum = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
    int mult = 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9;

    auto simpleReducer = [](int acc, int x)->int{return acc * x;};
    auto complexReducer = [](Result acc, int x)->Result{return Result{acc.sum + x, acc.mult * x};};

    result = algorithms::reduce(testContainer, simpleReducer, 1);
    EXPECT_EQ(mult, result);
    complexResult = algorithms::reduce(testContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(sum, complexResult.sum);
    EXPECT_EQ(mult, complexResult.mult);
    complexResult = algorithms::reduce(emptyContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(0, complexResult.sum);
    EXPECT_EQ(1, complexResult.mult);
}

TEST(AlgorithmsTest, reduceSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    struct Result {
        int sum;
        int mult;
    };
    int result;
    Result complexResult;
    int sum = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
    int mult = 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9;

    auto simpleReducer = [](int acc, int x)->int{return acc * x;};
    auto complexReducer = [](Result acc, int x)->Result{return Result{acc.sum + x, acc.mult * x};};

    result = algorithms::reduce(testContainer, simpleReducer, 1);
    EXPECT_EQ(mult, result);
    complexResult = algorithms::reduce(testContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(sum, complexResult.sum);
    EXPECT_EQ(mult, complexResult.mult);
    complexResult = algorithms::reduce(emptyContainer, complexReducer, Result{0, 1});
    EXPECT_EQ(0, complexResult.sum);
    EXPECT_EQ(1, complexResult.mult);
}



TEST(AlgorithmsTest, mapQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x)->int{return x * 2;};
    auto mapPredicate = [](int x)->QPair<int, bool>{return qMakePair(x * 2, !(x % 3));};

    resultQList = algorithms::map(emptyContainer, listPredicate, QList<int>());
    EXPECT_EQ(0, resultQList.size());
    resultVector = algorithms::map(emptyContainer, listPredicate, std::vector<int>());
    EXPECT_EQ(0u, resultVector.size());
    resultQList = algorithms::map(emptyContainer, listPredicate, QSet<int>()).toList();
    EXPECT_EQ(0, resultQList.size());
    resultVector = setToVector(algorithms::map(emptyContainer, listPredicate, std::set<int>()));
    EXPECT_EQ(0u, resultVector.size());
    resultQMap = algorithms::map(emptyContainer, mapPredicate, QMap<int, bool>());
    EXPECT_EQ(0, resultQMap.size());

    resultQList = algorithms::map(testContainer, listPredicate, QList<int>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, listPredicate, std::vector<int>());
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
}

TEST(AlgorithmsTest, mapQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x)->int{return x * 2;};
    auto mapPredicate = [](int x)->QPair<int, bool>{return qMakePair(x * 2, !(x % 3));};

    resultQList = algorithms::map(emptyContainer, listPredicate, QList<int>());
    EXPECT_EQ(0, resultQList.size());
    resultVector = algorithms::map(emptyContainer, listPredicate, std::vector<int>());
    EXPECT_EQ(0u, resultVector.size());
    resultQList = algorithms::map(emptyContainer, listPredicate, QSet<int>()).toList();
    EXPECT_EQ(0, resultQList.size());
    resultVector = setToVector(algorithms::map(emptyContainer, listPredicate, std::set<int>()));
    EXPECT_EQ(0u, resultVector.size());
    resultQMap = algorithms::map(emptyContainer, mapPredicate, QMap<int, bool>());
    EXPECT_EQ(0, resultQMap.size());

    resultQList = algorithms::map(testContainer, listPredicate, QList<int>());
    std::sort(resultQList.begin(), resultQList.end());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, listPredicate, std::vector<int>());
    std::sort(resultVector.begin(), resultVector.end());
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    std::sort(resultVector.begin(), resultVector.end());
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
}

TEST(AlgorithmsTest, mapQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true}, {2, false}, {3, true}, {4, false}, {5, true}, {6, false}, {7, true}, {8, false}, {9, true}};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x, bool y)->int{return y ? x : x * 2;};
    auto stableListPredicate = [](int x, bool)->int{return x * 2;};
    auto mapPredicate = [](int x, bool)->QPair<int, bool>{return qMakePair(x * 2, !(x % 3));};

    resultQList = algorithms::map(emptyContainer, listPredicate, QList<int>());
    EXPECT_EQ(0, resultQList.size());
    resultVector = algorithms::map(emptyContainer, listPredicate, std::vector<int>());
    EXPECT_EQ(0u, resultVector.size());
    resultQList = algorithms::map(emptyContainer, listPredicate, QSet<int>()).toList();
    EXPECT_EQ(0, resultQList.size());
    resultVector = setToVector(algorithms::map(emptyContainer, listPredicate, std::set<int>()));
    EXPECT_EQ(0u, resultVector.size());
    resultQMap = algorithms::map(emptyContainer, mapPredicate, QMap<int, bool>());
    EXPECT_EQ(0, resultQMap.size());

    resultQList = algorithms::map(testContainer, listPredicate, QList<int>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(testContainer[i] ? i : i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, listPredicate, std::vector<int>());
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(testContainer[i] ? i : i * 2, resultVector[i - 1]);
    resultQList = algorithms::map(testContainer, stableListPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, stableListPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
}

TEST(AlgorithmsTest, mapVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x)->int{return x * 2;};
    auto mapPredicate = [](int x)->QPair<int, bool>{return qMakePair(x * 2, !(x % 3));};

    resultQList = algorithms::map(emptyContainer, listPredicate, QList<int>());
    EXPECT_EQ(0, resultQList.size());
    resultVector = algorithms::map(emptyContainer, listPredicate, std::vector<int>());
    EXPECT_EQ(0u, resultVector.size());
    resultQList = algorithms::map(emptyContainer, listPredicate, QSet<int>()).toList();
    EXPECT_EQ(0, resultQList.size());
    resultVector = setToVector(algorithms::map(emptyContainer, listPredicate, std::set<int>()));
    EXPECT_EQ(0u, resultVector.size());
    resultQMap = algorithms::map(emptyContainer, mapPredicate, QMap<int, bool>());
    EXPECT_EQ(0, resultQMap.size());

    resultQList = algorithms::map(testContainer, listPredicate, QList<int>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, listPredicate, std::vector<int>());
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
}

TEST(AlgorithmsTest, mapSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x)->int{return x * 2;};
    auto mapPredicate = [](int x)->QPair<int, bool>{return qMakePair(x * 2, !(x % 3));};

    resultQList = algorithms::map(emptyContainer, listPredicate, QList<int>());
    EXPECT_EQ(0, resultQList.size());
    resultVector = algorithms::map(emptyContainer, listPredicate, std::vector<int>());
    EXPECT_EQ(0u, resultVector.size());
    resultQList = algorithms::map(emptyContainer, listPredicate, QSet<int>()).toList();
    EXPECT_EQ(0, resultQList.size());
    resultVector = setToVector(algorithms::map(emptyContainer, listPredicate, std::set<int>()));
    EXPECT_EQ(0u, resultVector.size());
    resultQMap = algorithms::map(emptyContainer, mapPredicate, QMap<int, bool>());
    EXPECT_EQ(0, resultQMap.size());

    resultQList = algorithms::map(testContainer, listPredicate, QList<int>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, listPredicate, std::vector<int>());
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i - 1]);
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
}
