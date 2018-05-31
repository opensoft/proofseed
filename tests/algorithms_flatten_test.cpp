// clazy:skip

#include "proofcore/proofalgorithms.h"

#include <QJsonArray>
#include <QList>
#include <QMap>
#include <QSet>
#include <QVector>

#include "gtest/test_global.h"

#include <set>
#include <vector>

using namespace Proof;

TEST(AlgorithmsTest, flattenQListQList)
{
    QList<QList<int>> testContainer = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}};
    QList<int> resultList = algorithms::flatten(testContainer, QList<int>());
    QVector<int> resultVector = algorithms::flatten(testContainer, QVector<int>());
    QSet<int> resultSet = algorithms::flatten(testContainer, QSet<int>());

    ASSERT_EQ(9, resultList.count());
    ASSERT_EQ(9, resultVector.count());
    ASSERT_EQ(9, resultSet.count());
    auto convertedSet = resultSet.toList();
    std::sort(convertedSet.begin(), convertedSet.end());
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(i + 1, resultList[i]);
        EXPECT_EQ(i + 1, resultVector[i]);
        EXPECT_EQ(i + 1, convertedSet[i]);
    }
}

TEST(AlgorithmsTest, flattenQVectorQList)
{
    QVector<QList<int>> testContainer = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}};
    QList<int> resultList = algorithms::flatten(testContainer, QList<int>());
    QVector<int> resultVector = algorithms::flatten(testContainer, QVector<int>());
    QSet<int> resultSet = algorithms::flatten(testContainer, QSet<int>());

    ASSERT_EQ(9, resultList.count());
    ASSERT_EQ(9, resultVector.count());
    ASSERT_EQ(9, resultSet.count());
    auto convertedSet = resultSet.toList();
    std::sort(convertedSet.begin(), convertedSet.end());
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(i + 1, resultList[i]);
        EXPECT_EQ(i + 1, resultVector[i]);
        EXPECT_EQ(i + 1, convertedSet[i]);
    }
}

TEST(AlgorithmsTest, flattenQVectorQSet)
{
    QVector<QSet<int>> testContainer = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}};
    QList<int> resultList = algorithms::flatten(testContainer, QList<int>());
    QVector<int> resultVector = algorithms::flatten(testContainer, QVector<int>());
    QSet<int> resultSet = algorithms::flatten(testContainer, QSet<int>());

    ASSERT_EQ(9, resultList.count());
    ASSERT_EQ(9, resultVector.count());
    ASSERT_EQ(9, resultSet.count());
    auto convertedSet = resultSet.toList();
    std::sort(convertedSet.begin(), convertedSet.end());
    std::sort(resultList.begin(), resultList.end());
    std::sort(resultVector.begin(), resultVector.end());
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(i + 1, resultList[i]);
        EXPECT_EQ(i + 1, resultVector[i]);
        EXPECT_EQ(i + 1, convertedSet[i]);
    }
}

TEST(AlgorithmsTest, flattenQListQListShort)
{
    QList<QList<int>> testContainer = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}};
    QList<int> resultList = algorithms::flatten(testContainer);

    ASSERT_EQ(9, resultList.count());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, resultList[i]);
}

TEST(AlgorithmsTest, flattenQVectorQListShort)
{
    QVector<QList<int>> testContainer = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}};
    QVector<int> resultVector = algorithms::flatten(testContainer);

    ASSERT_EQ(9, resultVector.count());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, resultVector[i]);
}

TEST(AlgorithmsTest, flattenQVectorQSetShort)
{
    QVector<QSet<int>> testContainer = {{1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}};
    QVector<int> resultVector = algorithms::flatten(testContainer);

    ASSERT_EQ(9, resultVector.count());
    std::sort(resultVector.begin(), resultVector.end());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i + 1, resultVector[i]);
}

TEST(AlgorithmsTest, flatFilterQListQList)
{
    QList<QList<int>> testContainer = {{0}, {1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}, {10, 11, 12, 13, 14}, {15, 16}};
    QList<int> resultList = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QList<int>());
    QVector<int> resultVector = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QVector<int>());
    QSet<int> resultSet = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QSet<int>());

    ASSERT_EQ(9, resultList.count());
    ASSERT_EQ(9, resultVector.count());
    ASSERT_EQ(9, resultSet.count());
    auto convertedSet = resultSet.toList();
    std::sort(convertedSet.begin(), convertedSet.end());
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(i * 2, resultList[i]);
        EXPECT_EQ(i * 2, resultVector[i]);
        EXPECT_EQ(i * 2, convertedSet[i]);
    }
}

TEST(AlgorithmsTest, flatFilterQVectorQList)
{
    QVector<QList<int>> testContainer = {{0}, {1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}, {10, 11, 12, 13, 14}, {15, 16}};
    QList<int> resultList = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QList<int>());
    QVector<int> resultVector = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QVector<int>());
    QSet<int> resultSet = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QSet<int>());

    ASSERT_EQ(9, resultList.count());
    ASSERT_EQ(9, resultVector.count());
    ASSERT_EQ(9, resultSet.count());
    auto convertedSet = resultSet.toList();
    std::sort(convertedSet.begin(), convertedSet.end());
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(i * 2, resultList[i]);
        EXPECT_EQ(i * 2, resultVector[i]);
        EXPECT_EQ(i * 2, convertedSet[i]);
    }
}

TEST(AlgorithmsTest, flatFilterQVectorQSet)
{
    QVector<QSet<int>> testContainer = {{0}, {1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}, {10, 11, 12, 13, 14}, {15, 16}};
    QList<int> resultList = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QList<int>());
    QVector<int> resultVector = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QVector<int>());
    QSet<int> resultSet = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); }, QSet<int>());

    ASSERT_EQ(9, resultList.count());
    ASSERT_EQ(9, resultVector.count());
    ASSERT_EQ(9, resultSet.count());
    auto convertedSet = resultSet.toList();
    std::sort(convertedSet.begin(), convertedSet.end());
    std::sort(resultList.begin(), resultList.end());
    std::sort(resultVector.begin(), resultVector.end());
    for (int i = 0; i < 9; ++i) {
        EXPECT_EQ(i * 2, resultList[i]);
        EXPECT_EQ(i * 2, resultVector[i]);
        EXPECT_EQ(i * 2, convertedSet[i]);
    }
}

TEST(AlgorithmsTest, flatFilterQListQListShort)
{
    QList<QList<int>> testContainer = {{0}, {1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}, {10, 11, 12, 13, 14}, {15, 16}};
    QList<int> resultList = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); });

    ASSERT_EQ(9, resultList.count());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * 2, resultList[i]);
}

TEST(AlgorithmsTest, flatFilterQVectorQListShort)
{
    QVector<QList<int>> testContainer = {{0}, {1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}, {10, 11, 12, 13, 14}, {15, 16}};
    QVector<int> resultVector = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); });

    ASSERT_EQ(9, resultVector.count());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i]);
}

TEST(AlgorithmsTest, flatFilterQVectorQSetShort)
{
    QVector<QSet<int>> testContainer = {{0}, {1, 2, 3}, {4, 5}, {}, {6, 7, 8, 9}, {10, 11, 12, 13, 14}, {15, 16}};
    QVector<int> resultVector = algorithms::flatFilter(testContainer, [](int x) { return !(x % 2); });

    ASSERT_EQ(9, resultVector.count());
    std::sort(resultVector.begin(), resultVector.end());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * 2, resultVector[i]);
}
