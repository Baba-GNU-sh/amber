#include <database/chunked_vector.hpp>
#include <gtest/gtest.h>

using namespace amber::database;

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

TEST(ChunkedVector, checkCapacity)
{
    ChunkedVector<int, 1024> data;
    data.push(1);
    ASSERT_EQ(data.capacity(), 1024);
    for (int i = 0; i < 1024; i++)
    {
        data.push(i);
    }
    ASSERT_EQ(data.capacity(), 2048);
}
