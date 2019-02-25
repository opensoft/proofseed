// clazy:skip

#include "proofseed/proofalgorithms.h"

#include "gtest/proof/test_global.h"

#include <QJsonArray>
#include <QList>
#include <QMap>
#include <QSet>
#include <QVector>

#include <set>
#include <vector>

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

    auto evenPredicate = [](int x) -> bool { return !(x % 2); };
    auto bigValuePredicate = [](int x) -> bool { return x > 42; };
    auto smallValuePredicate = [](int x) -> bool { return x < 42; };
    auto nonEqualPredicate = [](int x) -> bool { return x != 5; };

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

TEST(AlgorithmsTest, eraseIfQVector)
{
    QVector<int> emptyContainer;
    QVector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QVector<int> result;

    auto evenPredicate = [](int x) -> bool { return !(x % 2); };
    auto bigValuePredicate = [](int x) -> bool { return x > 42; };
    auto smallValuePredicate = [](int x) -> bool { return x < 42; };
    auto nonEqualPredicate = [](int x) -> bool { return x != 5; };

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

    auto evenPredicate = [](int x) -> bool { return !(x % 2); };
    auto bigValuePredicate = [](int x) -> bool { return x > 42; };
    auto smallValuePredicate = [](int x) -> bool { return x < 42; };
    auto nonEqualPredicate = [](int x) -> bool { return x != 5; };

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
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, static_cast<unsigned long>(result[i]));

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
    QMap<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                     {6, false}, {7, true},  {8, false}, {9, true}};
    QMap<int, bool> result;

    auto falseValuePredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(key);
        return !value;
    };
    auto bigValuePredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(value);
        return key > 42;
    };
    auto smallValuePredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(value);
        return key < 42;
    };
    auto nonEqualPredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(value);
        return key != 5;
    };

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
    QHash<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                      {6, false}, {7, true},  {8, false}, {9, true}};
    QHash<int, bool> result;

    auto falseValuePredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(key);
        return !value;
    };
    auto bigValuePredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(value);
        return key > 42;
    };
    auto smallValuePredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(value);
        return key < 42;
    };
    auto nonEqualPredicate = [](const auto &key, const auto &value) -> bool {
        Q_UNUSED(value);
        return key != 5;
    };

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

TEST(AlgorithmsTest, makeUniqueQVector)
{
    QVector<int> emptyContainer;
    QVector<int> testContainer = {1, 1, 2, 1, 3, 3, 3, 4, 1};
    QVector<int> result;

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

TEST(AlgorithmsTest, existsQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x % 2; };
    auto falsePredicate = [](int x) -> bool { return x > 42; };

    EXPECT_FALSE(algorithms::exists(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::exists(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::exists(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, existsQVector)
{
    QVector<int> emptyContainer;
    QVector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x % 2; };
    auto falsePredicate = [](int x) -> bool { return x > 42; };

    EXPECT_FALSE(algorithms::exists(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::exists(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::exists(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, existsQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x % 2; };
    auto falsePredicate = [](int x) -> bool { return x > 42; };

    EXPECT_FALSE(algorithms::exists(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::exists(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::exists(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, existsQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                     {6, false}, {7, true},  {8, false}, {9, true}};

    auto truePredicate = [](int, bool y) -> bool { return y; };
    auto falsePredicate = [](int x, bool) -> bool { return x > 42; };

    EXPECT_FALSE(algorithms::exists(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::exists(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::exists(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, existsVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x % 2; };
    auto falsePredicate = [](int x) -> bool { return x > 42; };

    EXPECT_FALSE(algorithms::exists(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::exists(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::exists(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, existsSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x % 2; };
    auto falsePredicate = [](int x) -> bool { return x > 42; };

    EXPECT_FALSE(algorithms::exists(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::exists(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::exists(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, forAllQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x < 42; };
    auto falsePredicate = [](int x) -> bool { return x == 5; };

    EXPECT_TRUE(algorithms::forAll(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::forAll(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::forAll(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, forAllQVector)
{
    QVector<int> emptyContainer;
    QVector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x < 42; };
    auto falsePredicate = [](int x) -> bool { return x == 5; };

    EXPECT_TRUE(algorithms::forAll(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::forAll(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::forAll(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, forAllQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x < 42; };
    auto falsePredicate = [](int x) -> bool { return x == 5; };

    EXPECT_TRUE(algorithms::forAll(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::forAll(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::forAll(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, forAllQMap)
{
    QMap<int, bool> emptyContainer;
    QMap<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                     {6, false}, {7, true},  {8, false}, {9, true}};

    auto truePredicate = [](int x, bool) -> bool { return x < 42; };
    auto falsePredicate = [](int, bool y) -> bool { return y; };

    EXPECT_TRUE(algorithms::forAll(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::forAll(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::forAll(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, forAllVector)
{
    std::vector<int> emptyContainer;
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x < 42; };
    auto falsePredicate = [](int x) -> bool { return x == 5; };

    EXPECT_TRUE(algorithms::forAll(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::forAll(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::forAll(testContainer, falsePredicate));
}

TEST(AlgorithmsTest, forAllSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    auto truePredicate = [](int x) -> bool { return x < 42; };
    auto falsePredicate = [](int x) -> bool { return x == 5; };

    EXPECT_TRUE(algorithms::forAll(emptyContainer, truePredicate));
    EXPECT_TRUE(algorithms::forAll(testContainer, truePredicate));
    EXPECT_FALSE(algorithms::forAll(testContainer, falsePredicate));
}
