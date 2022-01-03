#include "tsdb.hpp"
#include <gtest/gtest.h>

// TEST(TimeSeriesDB, addAndRetrieveGet)
// {
// 	TimeSeriesDB db;

// 	db.push("/foo", 0, 1.0);

// 	auto out = db.get("/foo", 0);
// 	EXPECT_FLOAT_EQ(out, 1.0);
// }

// TEST(TimeSeriesDB, getNonexistentSeries)
// {
// 	TimeSeriesDB db;

// 	EXPECT_THROW(db.get("/foo", 0), std::runtime_error);
// }

TEST(SparseTimeSeries, addAndRetrieveTwoSamples)
{
	SparseTimeSeries db;

	db.push(0, 1.0);
	db.push(1, 2.0);

	auto a = db.get(0);
	EXPECT_FLOAT_EQ(a, 1.0);

	auto b = db.get(1);
	EXPECT_FLOAT_EQ(b, 2.0);
}

TEST(SparseTimeSeries, interpolateTwoSamples)
{
	SparseTimeSeries db;

	db.push(0, 1.0);
	db.push(100, 2.0);

	auto a = db.get(50);
	EXPECT_FLOAT_EQ(a, 1.5);

	auto b = db.get(25);
	EXPECT_FLOAT_EQ(b, 1.25);

	auto c = db.get(75);
	EXPECT_FLOAT_EQ(c, 1.75);
}

TEST(SparseTimeSeries, outsideRange)
{
	SparseTimeSeries db;

	db.push(10, 1.0);
	db.push(15, 1.5);

	EXPECT_THROW(db.get(9), std::runtime_error);
	EXPECT_THROW(db.get(16), std::runtime_error);
}

TEST(SparseTimeSeries, checkSpan)
{
	SparseTimeSeries db;

	db.push(10, 1.0);
	db.push(15, 1.5);

	auto span = db.span();
	EXPECT_EQ(span.first, 10);
	EXPECT_EQ(span.second, 15);
}

TEST(SparseTimeSeries, getRange)
{
	SparseTimeSeries db;

	db.push(10, 1.0);
	db.push(20, 2.0);

	auto m = db.range(0, 30);
	EXPECT_EQ(std::distance(m.first, m.second), 2);
	EXPECT_FLOAT_EQ(m.first->value, 1.0);
	EXPECT_FLOAT_EQ((m.first + 1)->value, 2.0);
}

TEST(SparseTimeSeries, getMean)
{
	SparseTimeSeries db;

	db.push(10, 1.0);
	db.push(20, 2.0);

	EXPECT_FLOAT_EQ(db.mean(0, 15), 1.0);
	EXPECT_FLOAT_EQ(db.mean(15, 30), 2.0);
	EXPECT_FLOAT_EQ(db.mean(0, 30), 1.5);
	EXPECT_FLOAT_EQ(db.mean(0, 20), 1.5);
}

TEST(SparseTimeSeries, getMinMax)
{
	SparseTimeSeries db;

	db.push(10, 1.0);
	db.push(20, 2.0);

	EXPECT_FLOAT_EQ(db.max(0, 15), 1.0);
	EXPECT_FLOAT_EQ(db.max(15, 30), 2.0);
	EXPECT_FLOAT_EQ(db.max(0, 30), 2.0);
	EXPECT_FLOAT_EQ(db.max(0, 20), 2.0);
	EXPECT_FLOAT_EQ(db.max(10, 15), 1.0);

	EXPECT_FLOAT_EQ(db.min(0, 15), 1.0);
	EXPECT_FLOAT_EQ(db.min(15, 30), 2.0);
	EXPECT_FLOAT_EQ(db.min(0, 30), 1.0);
	EXPECT_FLOAT_EQ(db.min(0, 20), 1.0);
	EXPECT_FLOAT_EQ(db.min(10, 15), 1.0);
}

TEST(SparseTimeSeries, manySamples)
{
	SparseTimeSeries db;

	const Time SAMPLES = 1'000'000;

	std::vector<Sample> samples(SAMPLES);
	for (Time i = 0; i < SAMPLES; ++i)
	{
		samples[i].time = i;
		samples[i].value = i;
	}

	db.push_some(samples);

	const int STEP = 1'000;
	const int COLS = 1'000;
	auto output = db.mean(0, STEP, COLS);
	EXPECT_EQ(output.size(), COLS);
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
