#include <gtest/gtest.h>
#include <chunked_vector.hpp>

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
    ASSERT_EQ(data.at(10000-1), 10000-1);
}
