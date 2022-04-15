#include <chunked_vector.hpp>
#include <gtest/gtest.h>
#include <algorithm>

TEST(ChunkedVector, nullTest)
{
    ChunkedVector<double, 1024> data;
    ASSERT_EQ(data.size(), 0);

    data.push(123.456);
    ASSERT_EQ(data.size(), 1);
    ASSERT_DOUBLE_EQ(data.at(0), 123.456);

    data.push(543.343);
    ASSERT_EQ(data.size(), 2);
    ASSERT_DOUBLE_EQ(data.at(1), 543.343);
}

TEST(ChunkedVector, pushLots)
{
    ChunkedVector<int, 1024> data;
    for (int i = 0; i < 10000; i++)
    {
        data.push(i);
    }
    ASSERT_EQ(data.size(), 10000);
    ASSERT_EQ(data.at(0), 0);
    ASSERT_EQ(data.at(10000 - 1), 10000 - 1);
}

TEST(ChunkedVector, isEmpty)
{
    ChunkedVector<int, 1024> data;
    ASSERT_TRUE(data.empty());
    data.push(0);
    ASSERT_FALSE(data.empty());
}

TEST(ChunkedVector, squareBracketsOperator)
{
    ChunkedVector<int, 1024> data;
    data.push(1234);
    ASSERT_EQ(data[0], 1234);
}

TEST(ChunkedVector, outOfBoundsAccess)
{
    ChunkedVector<int, 1024> data;
    ASSERT_THROW(data.at(100), std::out_of_range);
}

TEST(ChunkedVector, iteration)
{
    ChunkedVector<int, 1024> data;
    data.push_back(0);
    data.push_back(1);
    data.push_back(2);

    int i = 0;
    for (auto iter = data.begin(); iter != data.end(); ++iter)
    {
        ASSERT_EQ(*iter, i++);
    }

    ASSERT_TRUE(std::binary_search(
        data.begin(), data.end(), 1, [](const auto &r1, const auto &r2) {
            return r1 < r2;
        }));

    ASSERT_FALSE(std::binary_search(
        data.begin(), data.end(), 5, [](const auto &r1, const auto &r2) {
            return r1 < r2;
        }));
}
