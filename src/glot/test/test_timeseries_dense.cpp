#include <gtest/gtest.h>
#include <timeseries_dense.hpp>

TEST(TimeSeriesDense, addAndRetrieveTwoSamples)
{
    TimeSeriesDense db(0.0, 1.0);

    db.push_sample(1.0);
    db.push_sample(2.0);

    auto a = db.get_sample(0.0, 1.0);
    EXPECT_FLOAT_EQ(a.average, 1.0);
    EXPECT_FLOAT_EQ(a.min, 1.0);
    EXPECT_FLOAT_EQ(a.max, 1.0);
}

TEST(TimeSeriesDense, oor)
{
    TimeSeriesDense db(0.0, 1.0);

    db.push_sample(1.0);
    db.push_sample(2.0);

    auto a = db.get_sample(-0.5, 1.0);
    EXPECT_FLOAT_EQ(a.timestamp, -0.5);
    EXPECT_FLOAT_EQ(a.average, 1.0);
    EXPECT_FLOAT_EQ(a.min, 1.0);
    EXPECT_FLOAT_EQ(a.max, 1.0);
}

TEST(TimeSeriesDense, larger)
{
    TimeSeriesDense db(0.0, 1.0);

    db.push_sample(1.0);
    db.push_sample(2.0);
    db.push_sample(3.0);

    auto a = db.get_sample(0.0, 3.0);
    EXPECT_FLOAT_EQ(a.average, 2.0);
    EXPECT_FLOAT_EQ(a.min, 1.0);
    EXPECT_FLOAT_EQ(a.max, 3.0);
}

TEST(TimeSeriesDense, push_loads)
{
    TimeSeriesDense db(0.0, 1.0);

    for (int i = 0; i < 16; i++)
    {
        db.push_sample(i);
    }
    auto a = db.get_sample(0.0, 16.0);
    EXPECT_FLOAT_EQ(a.average, 7.5);
}
