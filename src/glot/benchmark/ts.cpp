#include "benchmark/benchmark.h"
#include "timeseries.hpp"
#include <iostream>
#include <timeseries_dense.hpp>

void DenseTimeseriesPullSamples(benchmark::State &state)
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
BENCHMARK(DenseTimeseriesPullSamples);

BENCHMARK_MAIN();
