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
 * @brief The interface which all time series must implement.
 * A timeseries is a single numerical value which varies over time.
 * A timeseries object stores timeseries data, and provides bucketed access to these samples.
 */
struct TimeSeries
{
    virtual ~TimeSeries() = default;

    /**
     * @brief Pull information from the time series at a fixed interval.
     *
     * @param samples Where to put the samples.
     * @param timestamp_start Timestamp of the first sample.
     * @param bin_width The width of each bin in seconds.
     * @param num_samples The number of samples to pull out.
     */
    virtual std::size_t get_samples(TSSample *samples,
                                    double timestamp_start,
                                    double bin_width,
                                    std::size_t num_samples) const = 0;

    /**
     * @brief Returns a single sample.
     *
     * @param timestamp
     * @return TSSample
     */
    virtual TSSample get_sample(double timestamp, double bin_width) const = 0;

    /**
     * @brief Get the timestamps of the oldest and newest samples.
     */
    virtual std::pair<double, double> get_span() const = 0;

    /**
     * @brief Gets the amount of memory used by the timeseries in bytes.
     */
    virtual std::size_t memory_usage() const = 0;

    /**
     * @brief Get the total number of samples in this timeseries.
     */
    virtual std::size_t size() const = 0;
};
