#include "timeseries.hpp"
#include <benchmark/benchmark.h>
#include <iostream>
#include <timeseries_dense.hpp>

static void TimeseriesDense_Push(benchmark::State &state)
{
    for (auto _ : state)
    {
        TimeSeriesDense ts(0, 1.0);
        for (int i = 0; i < state.range(0); i++)
        {
            ts.push_sample(i);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(state.range(0)));
}
BENCHMARK(TimeseriesDense_Push)
    ->Unit(benchmark::kMillisecond)
    ->ArgName("samples")
    ->Arg(1'000'000)
    ->Arg(10'000'000)
    ->Arg(100'000'000);

static void TimeseriesDense_Reduce(benchmark::State &state)
{
    const int TOTAL_SAMPLES = state.range(0);
    const int TOTAL_BINS = state.range(1);

    // TODO is this step excluded from timing?
    TimeSeriesDense ts(0, 1.0);
    for (int i = 0; i < TOTAL_SAMPLES; i++)
    {
        ts.push_sample(i);
    }

    for (auto _ : state)
    {
        TSSample sample[TOTAL_BINS];
        benchmark::DoNotOptimize(
            ts.get_samples(&sample[0], 0.0, TOTAL_SAMPLES / TOTAL_BINS, TOTAL_BINS));
    }
}
BENCHMARK(TimeseriesDense_Reduce)
    ->Unit(benchmark::kMillisecond)
    ->ArgNames({"samples", "bins"})
    ->Args({1'000'000, 1'000})
    ->Args({10'000'000, 1'000})
    ->Args({100'000'000, 1'000});

BENCHMARK_MAIN();
