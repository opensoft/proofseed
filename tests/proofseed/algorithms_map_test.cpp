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

TEST(AlgorithmsTest, mapQByteArray)
{
    QByteArray emptyContainer;
    QByteArray testContainer = "123456789";
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](char x) -> int { return (x - '1' + 1) * 2; };
    auto mapPredicate = [](char x) -> QPair<int, bool> { return qMakePair((x - '1' + 1) * 2, !((x - '1' + 1) % 3)); };

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
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
}

TEST(AlgorithmsTest, mapQJsonArray)
{
    QJsonArray emptyContainer;
    QJsonArray testContainer = {QJsonValue(1), QJsonValue(2), QJsonValue(3), QJsonValue(4), QJsonValue(5),
                                QJsonValue(6), QJsonValue(7), QJsonValue(8), QJsonValue(9)};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](const QJsonValue &x) -> int { return x.toInt() * 2; };
    auto mapPredicate = [](const QJsonValue &x) -> QPair<int, bool> {
        return qMakePair(x.toInt() * 2, !(x.toInt() % 3));
    };

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
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
}

TEST(AlgorithmsTest, mapQList)
{
    QList<int> emptyContainer;
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x) -> int { return x * 2; };
    auto indicedPredicate = [](long long index, int x) -> int { return x * static_cast<int>(index); };
    auto mapPredicate = [](int x) -> QPair<int, bool> { return qMakePair(x * 2, !(x % 3)); };

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
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }

    resultQList = algorithms::map(testContainer, indicedPredicate, QList<int>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), resultQList[i]);
    resultVector = algorithms::map(testContainer, indicedPredicate, std::vector<int>());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), static_cast<unsigned long>(resultVector[i]));
    resultQList = algorithms::map(testContainer, indicedPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), resultQList[i]);
    resultVector = setToVector(algorithms::map(testContainer, indicedPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), static_cast<unsigned long>(resultVector[i]));
}

TEST(AlgorithmsTest, mapQVector)
{
    QVector<int> emptyContainer;
    QVector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x) -> int { return x * 2; };
    auto indicedPredicate = [](long long index, int x) -> int { return x * static_cast<int>(index); };
    auto mapPredicate = [](int x) -> QPair<int, bool> { return qMakePair(x * 2, !(x % 3)); };

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
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }

    resultQList = algorithms::map(testContainer, indicedPredicate, QList<int>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), resultQList[i]);
    resultVector = algorithms::map(testContainer, indicedPredicate, std::vector<int>());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), static_cast<unsigned long>(resultVector[i]));
    resultQList = algorithms::map(testContainer, indicedPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), resultQList[i]);
    resultVector = setToVector(algorithms::map(testContainer, indicedPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), static_cast<unsigned long>(resultVector[i]));
}

TEST(AlgorithmsTest, mapQSet)
{
    QSet<int> emptyContainer;
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x) -> int { return x * 2; };
    auto indicedPredicate = [](long long, int x) -> int { return x * 2; };
    auto mapPredicate = [](int x) -> QPair<int, bool> { return qMakePair(x * 2, !(x % 3)); };

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
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    std::sort(resultVector.begin(), resultVector.end());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
    resultQList = algorithms::map(testContainer, indicedPredicate, QList<int>());
    std::sort(resultQList.begin(), resultQList.end());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, indicedPredicate, std::vector<int>());
    std::sort(resultVector.begin(), resultVector.end());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, indicedPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, indicedPredicate, std::set<int>()));
    std::sort(resultVector.begin(), resultVector.end());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
}

TEST(AlgorithmsTest, mapQMap)
{
    QMap<long long, bool> emptyContainer;
    QMap<long long, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                           {6, false}, {7, true},  {8, false}, {9, true}};
    QList<long long> resultQList;
    std::vector<long long> resultVector;
    QMap<long long, bool> resultQMap;

    auto listPredicate = [](long long x, bool y) -> long long { return y ? x : x * 2; };
    auto stableListPredicate = [](long long x, bool) -> long long { return x * 2; };
    auto mapPredicate = [](long long x, bool) -> QPair<long long, bool> { return qMakePair(x * 2, !(x % 3)); };

    resultQList = algorithms::map(emptyContainer, listPredicate, QList<long long>());
    EXPECT_EQ(0, resultQList.size());
    resultVector = algorithms::map(emptyContainer, listPredicate, std::vector<long long>());
    EXPECT_EQ(0u, resultVector.size());
    resultQList = algorithms::map(emptyContainer, listPredicate, QSet<long long>()).toList();
    EXPECT_EQ(0, resultQList.size());
    resultVector = setToVector(algorithms::map(emptyContainer, listPredicate, std::set<long long>()));
    EXPECT_EQ(0u, resultVector.size());
    resultQMap = algorithms::map(emptyContainer, mapPredicate, QMap<long long, bool>());
    EXPECT_EQ(0, resultQMap.size());

    resultQList = algorithms::map(testContainer, listPredicate, QList<long long>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(testContainer[i] ? i : i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, listPredicate, std::vector<long long>());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(testContainer[static_cast<long long>(i)] ? i : i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, stableListPredicate, QSet<long long>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, stableListPredicate, std::set<long long>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<long long, bool>());
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

    auto listPredicate = [](int x) -> int { return x * 2; };
    auto indicedPredicate = [](long long index, int x) -> int { return x * static_cast<int>(index); };
    auto mapPredicate = [](int x) -> QPair<int, bool> { return qMakePair(x * 2, !(x % 3)); };

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
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }

    resultQList = algorithms::map(testContainer, indicedPredicate, QList<int>());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), resultQList[i]);
    resultVector = algorithms::map(testContainer, indicedPredicate, std::vector<int>());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), static_cast<unsigned long>(resultVector[i]));
    resultQList = algorithms::map(testContainer, indicedPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), resultQList[i]);
    resultVector = setToVector(algorithms::map(testContainer, indicedPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i * (i + 1), static_cast<unsigned long>(resultVector[i]));
}

TEST(AlgorithmsTest, mapSet)
{
    std::set<int> emptyContainer;
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    QList<int> resultQList;
    std::vector<int> resultVector;
    QMap<int, bool> resultQMap;

    auto listPredicate = [](int x) -> int { return x * 2; };
    auto indicedPredicate = [](long long, int x) -> int { return x * 2; };
    auto mapPredicate = [](int x) -> QPair<int, bool> { return qMakePair(x * 2, !(x % 3)); };

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
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, listPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, listPredicate, std::set<int>()));
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQMap = algorithms::map(testContainer, mapPredicate, QMap<int, bool>());
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }
    resultQList = algorithms::map(testContainer, indicedPredicate, QList<int>());
    std::sort(resultQList.begin(), resultQList.end());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = algorithms::map(testContainer, indicedPredicate, std::vector<int>());
    std::sort(resultVector.begin(), resultVector.end());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
    resultQList = algorithms::map(testContainer, indicedPredicate, QSet<int>()).toList();
    ASSERT_EQ(9, resultQList.size());
    std::sort(resultQList.begin(), resultQList.end());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
    resultVector = setToVector(algorithms::map(testContainer, indicedPredicate, std::set<int>()));
    std::sort(resultVector.begin(), resultVector.end());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));
}

TEST(AlgorithmsTest, mapQListShort)
{
    QList<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    QList<int> resultQList = algorithms::map(testContainer, [](int x) { return x * 2; });
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);

    QList<double> resultQList2 = algorithms::map(testContainer, [](int x) { return x * 2.0; });
    ASSERT_EQ(9, resultQList2.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_DOUBLE_EQ(i * 2, resultQList2[i - 1]);

    QList<long long> resultQList3 = algorithms::map(testContainer, [](long long index, int x) { return x * index; });
    ASSERT_EQ(9, resultQList3.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * testContainer[i], resultQList3[i]);
}

TEST(AlgorithmsTest, mapQVectorShort)
{
    QVector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    QVector<int> resultQVector = algorithms::map(testContainer, [](int x) { return x * 2; });
    ASSERT_EQ(9, resultQVector.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQVector[i - 1]);

    QVector<double> resultQVector2 = algorithms::map(testContainer, [](int x) { return x * 2.0; });
    ASSERT_EQ(9, resultQVector2.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_DOUBLE_EQ(i * 2, resultQVector2[i - 1]);

    QVector<long long> resultQVector3 = algorithms::map(testContainer, [](long long index, int x) { return x * index; });
    ASSERT_EQ(9, resultQVector3.size());
    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(i * testContainer[i], resultQVector3[i]);
}

TEST(AlgorithmsTest, mapQSetShort)
{
    QSet<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    QSet<int> resultQSet = algorithms::map(testContainer, [](int x) { return x * 2; });
    QList<int> resultQList = resultQSet.toList();
    std::sort(resultQList.begin(), resultQList.end());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);

    QSet<int> resultQSet2 = algorithms::map(testContainer, [](long long, int x) { return x * 2; });
    resultQList = resultQSet.toList();
    std::sort(resultQList.begin(), resultQList.end());
    ASSERT_EQ(9, resultQList.size());
    for (int i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, resultQList[i - 1]);
}

TEST(AlgorithmsTest, mapQMapShort)
{
    QMap<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                     {6, false}, {7, true},  {8, false}, {9, true}};

    QMap<int, bool> resultQMap = algorithms::map(testContainer, [](int x, bool) { return qMakePair(x * 2, !(x % 3)); });
    ASSERT_EQ(9, resultQMap.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQMap.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQMap[i * 2]);
    }

    QMap<QString, int> resultQMap2 = algorithms::map(testContainer,
                                                     [](int x, bool) { return qMakePair(QString::number(x), x % 3); });
    ASSERT_EQ(9, resultQMap2.size());
    for (int i = 1; i <= 9; ++i) {
        QString key = QString::number(i);
        EXPECT_TRUE(resultQMap2.contains(key));
        EXPECT_EQ(i % 3, resultQMap2[key]);
    }
}

TEST(AlgorithmsTest, mapQHashShort)
{
    QHash<int, bool> testContainer = {{1, true},  {2, false}, {3, true},  {4, false}, {5, true},
                                      {6, false}, {7, true},  {8, false}, {9, true}};

    QHash<int, bool> resultQHash = algorithms::map(testContainer, [](int x, bool) { return qMakePair(x * 2, !(x % 3)); });
    ASSERT_EQ(9, resultQHash.size());
    for (int i = 1; i <= 9; ++i) {
        EXPECT_TRUE(resultQHash.contains(i * 2));
        EXPECT_EQ(!(i % 3), resultQHash[i * 2]);
    }

    QHash<QString, int> resultQHash2 = algorithms::map(testContainer,
                                                       [](int x, bool) { return qMakePair(QString::number(x), x % 3); });
    ASSERT_EQ(9, resultQHash2.size());
    for (int i = 1; i <= 9; ++i) {
        QString key = QString::number(i);
        EXPECT_TRUE(resultQHash2.contains(key));
        EXPECT_EQ(i % 3, resultQHash2[key]);
    }
}

TEST(AlgorithmsTest, mapVectorShort)
{
    std::vector<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::vector<int> resultVector = algorithms::map(testContainer, [](int x) { return x * 2; });
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));

    std::vector<double> resultVector2 = algorithms::map(testContainer, [](int x) { return x * 2.0; });
    ASSERT_EQ(9u, resultVector2.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_DOUBLE_EQ(i * 2.0, static_cast<unsigned long>(resultVector2[i - 1]));

    std::vector<long long> resultVector3 = algorithms::map(testContainer,
                                                           [](long long index, int x) { return x * index; });
    ASSERT_EQ(9u, resultVector3.size());
    for (size_t i = 0; i < 9; ++i)
        EXPECT_EQ(i * static_cast<unsigned int>(testContainer[i]), static_cast<unsigned int>(resultVector3[i]));
}

TEST(AlgorithmsTest, mapSetShort)
{
    std::set<int> testContainer = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::set<int> resultSet = algorithms::map(testContainer, [](int x) { return x * 2; });
    std::vector<int> resultVector(resultSet.begin(), resultSet.end());
    ASSERT_EQ(9u, resultVector.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector[i - 1]));

    std::set<int> resultQSet2 = algorithms::map(testContainer, [](long long, int x) { return x * 2; });
    std::vector<int> resultVector2(resultSet.begin(), resultSet.end());
    ASSERT_EQ(9u, resultVector2.size());
    for (size_t i = 1; i <= 9; ++i)
        EXPECT_EQ(i * 2, static_cast<unsigned long>(resultVector2[i - 1]));
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
