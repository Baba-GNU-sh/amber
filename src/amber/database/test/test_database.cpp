#include <database/database.hpp>
#include <database/timeseries_dense.hpp>
#include <gtest/gtest.h>

TEST(Database, putAndGetTimeseries)
{
    Database db;
    db.register_timeseries("a", std::make_shared<TimeSeriesDense>(0.0, 1.0));
    ASSERT_EQ(db.data().size(), 1);
    db.data().at("a");
}

TEST(Database, countSamples)
{
    Database db;
    auto a = std::make_shared<TimeSeriesDense>(0.0, 1.0);
    db.register_timeseries("a", a);

    a->push_sample(123);
    ASSERT_EQ(db.num_samples(), 1);
}

TEST(Database, memoryUsage)
{
    Database db;
    auto a = std::make_shared<TimeSeriesDense>(0.0, 1.0);
    db.register_timeseries("a", a);

    for (int i = 0; i < 128; i++)
    {
        a->push_sample(123);
    }

    ASSERT_GE(db.memory_usage(), sizeof(double) * 128);
}
