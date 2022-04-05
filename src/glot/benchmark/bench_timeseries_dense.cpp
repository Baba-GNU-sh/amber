#include "benchmark/benchmark.h"
#include "timeseries.hpp"
#include <iostream>
#include <timeseries_dense.hpp>

void DenseTimeseries_Summarize_1M_1K(benchmark::State &state)
{
    const int TOTAL_SAMPLES = 1'000'000;
    const int TOTAL_BINS = 1'000;

    TimeSeriesDense ts(0, 1.0);
    for (int i = 0; i < TOTAL_SAMPLES; i++)
        ts.push_sample(i);
    TSSample sample[TOTAL_BINS];
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(
            ts.get_samples(&sample[0], 0.0, TOTAL_SAMPLES / TOTAL_BINS, TOTAL_BINS));
    }
}
BENCHMARK(DenseTimeseries_Summarize_1M_1K)->Unit(benchmark::kMillisecond);

void DenseTimeseries_Summarize_10M_1K(benchmark::State &state)
{
    const int TOTAL_SAMPLES = 10'000'000;
    const int TOTAL_BINS = 1'000;

    TimeSeriesDense ts(0, 1.0);
    for (int i = 0; i < TOTAL_SAMPLES; i++)
        ts.push_sample(i);
    TSSample sample[TOTAL_BINS];
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(
            ts.get_samples(&sample[0], 0.0, TOTAL_SAMPLES / TOTAL_BINS, TOTAL_BINS));
    }
}
BENCHMARK(DenseTimeseries_Summarize_10M_1K)->Unit(benchmark::kMillisecond);

void DenseTimeseries_Summarize_100M_1K(benchmark::State &state)
{
    const int TOTAL_SAMPLES = 100'000'000;
    const int TOTAL_BINS = 1'000;

    TimeSeriesDense ts(0, 1.0);
    for (int i = 0; i < TOTAL_SAMPLES; i++)
        ts.push_sample(i);
    TSSample sample[TOTAL_BINS];
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(
            ts.get_samples(&sample[0], 0.0, TOTAL_SAMPLES / TOTAL_BINS, TOTAL_BINS));
    }
}
BENCHMARK(DenseTimeseries_Summarize_100M_1K)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
