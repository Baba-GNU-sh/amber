#include "timeseries.hpp"
#include <benchmark/benchmark.h>
#include <iostream>
#include <timeseries_dense.hpp>

#include <glm/matrix.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

static void TimeseriesDense_Push(benchmark::State &state)
{
    const int TOTAL_SAMPLES = state.range(0);

    for (auto _ : state)
    {
        TimeSeriesDense ts(0, 1.0);
        for (int i = 0; i < TOTAL_SAMPLES; i++)
        {
            ts.push_sample(i);
        }
    }
    auto items = int64_t(state.iterations()) * int64_t(state.range(0));
    state.counters["samples/sec"] =
        benchmark::Counter(static_cast<double>(items), benchmark::Counter::kIsRate);
}
BENCHMARK(TimeseriesDense_Push)
    ->Unit(benchmark::kMillisecond)
    ->ArgName("samples")
    ->Arg(1'000'000)
    ->Arg(10'000'000)
    ->Arg(100'000'000);

static void TimeseriesDense_Init(benchmark::State &state)
{
    const int TOTAL_SAMPLES = state.range(0);

    for (auto _ : state)
    {
        std::vector<double> data(TOTAL_SAMPLES, 0.0);
        TimeSeriesDense ts(0, 1.0, data);
    }
    auto items = int64_t(state.iterations()) * int64_t(state.range(0));
    state.counters["samples/sec"] =
        benchmark::Counter(static_cast<double>(items), benchmark::Counter::kIsRate);
}
BENCHMARK(TimeseriesDense_Init)
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
    std::vector<double> data(TOTAL_SAMPLES, 0.0);
    TimeSeriesDense ts(0, 1.0, data);

    for (auto _ : state)
    {
        TSSample sample[TOTAL_BINS];
        benchmark::DoNotOptimize(
            ts.get_samples(&sample[0], 0.0, TOTAL_SAMPLES / TOTAL_BINS, TOTAL_BINS));
    }
    auto items = int64_t(state.iterations()) * int64_t(TOTAL_BINS);
    state.counters["bins/sec"] =
        benchmark::Counter(static_cast<double>(items), benchmark::Counter::kIsRate);
}
BENCHMARK(TimeseriesDense_Reduce)
    ->Unit(benchmark::kMicrosecond)
    ->ArgNames({"samples", "bins"})
    ->Args({1'000'000, 1'000})
    ->Args({10'000'000, 1'000})
    ->Args({100'000'000, 1'000});

static void GLMInverse(benchmark::State &state)
{
    const glm::mat3 identity(1.0f);
    auto vp_matrix = glm::scale(identity, glm::vec2(800 / 2, -600 / 2));
    vp_matrix = glm::translate(vp_matrix, glm::vec2(1, -1));

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(glm::inverse(vp_matrix));
    }
}
BENCHMARK(GLMInverse);

BENCHMARK_MAIN();
