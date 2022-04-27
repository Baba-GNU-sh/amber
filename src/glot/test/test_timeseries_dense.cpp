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

TEST(TimeSeriesDense, InitialValues)
{
    std::vector<double> initial_values = {0, 1, 2, 3};
    TimeSeriesDense db(0.0, 1.0, initial_values);

    auto a = db.get_sample(0.0, 1.0);
    EXPECT_FLOAT_EQ(a.average, 0.0);
}

TEST(TimeSeriesDense, Oversampled)
{
    std::vector<double> initial_values = {0, 1, 2, 3};
    TimeSeriesDense db(0.0, 1.0, initial_values);

    TSSample samples[6];
    auto n_samples = db.get_samples(samples, 1.0, 0.5, 6);

    EXPECT_EQ(n_samples, 6);
    EXPECT_FLOAT_EQ(samples[0].average, 1.0);
    EXPECT_FLOAT_EQ(samples[1].average, 1.0);
    EXPECT_FLOAT_EQ(samples[2].average, 2.0);
    EXPECT_FLOAT_EQ(samples[3].average, 2.0);
    EXPECT_FLOAT_EQ(samples[4].average, 3.0);
    EXPECT_FLOAT_EQ(samples[5].average, 3.0);
}

TEST(TimeSeriesDense, OffFrontEnd)
{
    std::vector<double> initial_values = {0, 1, 2, 3};
    TimeSeriesDense db(0.0, 1.0, initial_values);

    TSSample samples[5];
    auto n_samples = db.get_samples(samples, -100.0, 1.0, 5);
    EXPECT_EQ(n_samples, 0);
}

TEST(TimeSeriesDense, OffBackEnd)
{
    std::vector<double> initial_values = {0, 1, 2, 3};
    TimeSeriesDense db(0.0, 1.0, initial_values);

    TSSample samples[5];
    auto n_samples = db.get_samples(samples, 100.0, 1.0, 5);
    EXPECT_EQ(n_samples, 0);
}

TEST(TimeSeriesDense, EmptySet)
{
    TimeSeriesDense db(0.0, 1.0);

    TSSample samples[5];
    auto n_samples = db.get_samples(samples, 100.0, 1.0, 5);
    EXPECT_EQ(n_samples, 0);
}

TEST(TimeSeriesDense, MemoryUsage)
{
    // We don't really care how much memory the thing is using, as long as it looks sane
    std::vector<double> initial_values = {0, 1, 2, 3};
    TimeSeriesDense db(0.0, 1.0, initial_values);
    ASSERT_GT(db.memory_usage(), initial_values.size() * sizeof(double));
}

TEST(TimeSeriesDense, CheckSize)
{
    std::vector<double> initial_values = {0, 1, 2, 3};
    TimeSeriesDense db(0.0, 1.0, initial_values);
    ASSERT_EQ(db.size(), initial_values.size());
}
