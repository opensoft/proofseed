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

TEST(AlgorithmsTest, mapQListInPlace)
{
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    algorithms::mapInPlace(testContainer, [](int x) { return x * 2; });
    ASSERT_EQ(9, testContainer.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, testContainer[i - 1]);

    algorithms::mapInPlace(testContainer, [](long long i, int x) { return x + i + 1; });
    ASSERT_EQ(9, testContainer.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2 + i, testContainer[i - 1]);
}

TEST(AlgorithmsTest, mapQVectorInPlace)
{
    QVector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    algorithms::mapInPlace(testContainer, [](int x) { return x * 2; });
    ASSERT_EQ(9, testContainer.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, testContainer[i - 1]);

    algorithms::mapInPlace(testContainer, [](long long i, int x) { return x + i + 1; });
    ASSERT_EQ(9, testContainer.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2 + i, testContainer[i - 1]);
}

TEST(AlgorithmsTest, mapQMapInPlace)
{
    QMap<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                     {6, false}, {7, true},  {8, false}, {9, true}};

    algorithms::mapInPlace(testContainer, [](int x, bool) -> bool { return !(x % 3); });
    ASSERT_EQ(9, testContainer.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(!(i % 3), testContainer[i]);
}

TEST(AlgorithmsTest, mapQHashInPlace)
{
    QHash<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                      {6, false}, {7, true},  {8, false}, {9, true}};

    algorithms::mapInPlace(testContainer, [](int x, bool) -> bool { return !(x % 3); });
    ASSERT_EQ(9, testContainer.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(!(i % 3), testContainer[i]);
}

TEST(AlgorithmsTest, mapVectorInPlace)
{
    std::vector<unsigned int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    algorithms::mapInPlace(testContainer, [](int x) { return x * 2; });
    ASSERT_EQ(9u, testContainer.size());
    for (unsigned int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, testContainer[i - 1]);

    algorithms::mapInPlace(testContainer, [](long long i, int x) { return x + i + 1; });
    ASSERT_EQ(9u, testContainer.size());
    for (unsigned int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2 + i, testContainer[i - 1]);
}

TEST(AlgorithmsTest, mapMapInPlace)
{
    std::map<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                         {6, false}, {7, true},  {8, false}, {9, true}};

    algorithms::mapInPlace(testContainer, [](int x, bool) -> bool { return !(x % 3); });
    ASSERT_EQ(9u, testContainer.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(!(i % 3), testContainer[i]);
}

TEST(AlgorithmsTest, toSet)
{
    QList<int> qList = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QVector<int> qVector = qList.toVector();
    QSet<int> qSet = qList.toSet();
    std::vector<int> stdVector = qVector.toStdVector();

    QSet<int> rawResult;
    QList<int> sortedResult;

    rawResult = algorithms::toSet(qList);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, sortedResult[i - 1]);

    rawResult = algorithms::toSet(qVector);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, sortedResult[i - 1]);

    rawResult = algorithms::toSet(qSet);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, sortedResult[i - 1]);

    rawResult = algorithms::toSet(stdVector);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, sortedResult[i - 1]);
}

TEST(AlgorithmsTest, toVector)
{
    QList<int> qList = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QVector<int> qVector = qList.toVector();
    QSet<int> qSet = qList.toSet();
    std::vector<int> stdVector = qVector.toStdVector();

    QVector<int> result;

    result = algorithms::toVector(qList);
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toVector(qVector);
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toVector(qSet);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toVector(stdVector);
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);
}

TEST(AlgorithmsTest, toList)
{
    QList<int> qList = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QVector<int> qVector = qList.toVector();
    QSet<int> qSet = qList.toSet();
    std::vector<int> stdVector = qVector.toStdVector();

    QList<int> result;

    result = algorithms::toList(qList);
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toList(qVector);
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toList(qSet);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toList(stdVector);
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);
}

TEST(AlgorithmsTest, toKeysSet)
{
    QMap<int, int> qMap = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    QHash<int, int> qHash = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    std::map<int, int> stdMap = qMap.toStdMap();

    QSet<int> rawResult;
    QList<int> sortedResult;

    rawResult = algorithms::toKeysSet(qMap);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, sortedResult[i - 1]);

    rawResult = algorithms::toKeysSet(qHash);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, sortedResult[i - 1]);

    rawResult = algorithms::toKeysSet(stdMap);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, sortedResult[i - 1]);
}

TEST(AlgorithmsTest, toKeysVector)
{
    QMap<int, int> qMap = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    QHash<int, int> qHash = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    std::map<int, int> stdMap = qMap.toStdMap();

    QVector<int> result;

    result = algorithms::toKeysVector(qMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toKeysVector(qHash);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toKeysVector(stdMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);
}

TEST(AlgorithmsTest, toKeysList)
{
    QMap<int, int> qMap = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    QHash<int, int> qHash = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    std::map<int, int> stdMap = qMap.toStdMap();

    QList<int> result;

    result = algorithms::toKeysList(qMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toKeysList(qHash);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);

    result = algorithms::toKeysList(stdMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i, result[i - 1]);
}

TEST(AlgorithmsTest, toValuesSet)
{
    QMap<int, int> qMap = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    QHash<int, int> qHash = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    std::map<int, int> stdMap = qMap.toStdMap();

    QSet<int> rawResult;
    QList<int> sortedResult;

    rawResult = algorithms::toValuesSet(qMap);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, sortedResult[i - 1]);

    rawResult = algorithms::toValuesSet(qHash);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, sortedResult[i - 1]);

    rawResult = algorithms::toValuesSet(stdMap);
    sortedResult = rawResult.toList();
    std::sort(sortedResult.begin(), sortedResult.end());
    ASSERT_EQ(9, sortedResult.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, sortedResult[i - 1]);
}

TEST(AlgorithmsTest, toValuesVector)
{
    QMap<int, int> qMap = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    QHash<int, int> qHash = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    std::map<int, int> stdMap = qMap.toStdMap();

    QVector<int> result;

    result = algorithms::toValuesVector(qMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, result[i - 1]);

    result = algorithms::toValuesVector(qHash);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, result[i - 1]);

    result = algorithms::toValuesVector(stdMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, result[i - 1]);
}

TEST(AlgorithmsTest, toValuesList)
{
    QMap<int, int> qMap = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    QHash<int, int> qHash = {{1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}};
    std::map<int, int> stdMap = qMap.toStdMap();

    QList<int> result;

    result = algorithms::toValuesList(qMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, result[i - 1]);

    result = algorithms::toValuesList(qHash);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, result[i - 1]);

    result = algorithms::toValuesList(stdMap);
    std::sort(result.begin(), result.end());
    ASSERT_EQ(9, result.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i + 10, result[i - 1]);
}
