#pragma once

#include <cstddef>
#include <utility>

struct TSSample
{
    float timestamp;
    float average;
    float min;
    float max;
};

/**
 * @brief Defines the interface which all time series must implement.
 */
class TimeSeries
{
  public:
    /**
     * @brief Gets binned samples from the time series.
     *
     * @param samples Where to put the samples.
     * @param timestamp_start The first timestamp from which to start reading from.
     * @param bin_width The width of each bin in seconds.
     * @param num_samples The number of samples to pull out.
     */
    virtual std::size_t get_samples(TSSample *samples, double timestamp_start, double bin_width, std::size_t num_samples) const = 0;

    /**
     * @brief Get the oldest and newest samples.
     */
    virtual std::pair<double, double> get_span() const = 0;
};