#pragma once

#include <mutex>
#include <tuple>
#include <utility>
#include <vector>

#include "chunked_vector.hpp"
#include "timeseries.hpp"

struct DataStore
{
    double sum;
    double min;
    double max;
};

/**
 * @brief A timeseries implementaion for fixed-rate data sources.
 *
 * Use this timeseries implementation when your data source produces samples at a fixed
 * interval. It writes samples in a densely packed array, which is preferrable to a sparse time
 * series as it uses less storage because samples don't require individual timestamping.

 * This implemenation creates mip-maps on the fly, which results in taking up three times a much
 * space as the raw samples, but makes reduce operations significantly cheaper.
 *
 * Actually, this implementation actually has a flaw which ends up taking up 6 time as much space as
 * raw samples, but this is an oversight and has lots of duplicate data!
 *
 * Without mipmaps: Complexity = O(N)
 * With mipmaps: Worst case complexity = O(2*log2(N))
 * Where N in the number of total samples required to be reduced.
 */
class TimeSeriesDense : public TimeSeries
{
  public:
    /**
     * @brief Create an empty timeseries.
     *
     * @param initial_timestamp Timestamp of the first sample.
     * @param interval Time interval between each sample.
     */
    TimeSeriesDense(double initial_timestamp, double interval);

    /**
     * @brief Create a timeseries initialized from an array of data.
     *
     * @param initial_timestamp Timestamp of the first sample.
     * @param interval Time interval between each sample.
     * @param init Initial data which which to fill the timeseries.
     */
    TimeSeriesDense(double initial_timestamp, double interval, const std::vector<double> init);

    virtual ~TimeSeriesDense() = default;

    /**
     * @brief Assembles groups of samples into buckets, and procudes the average, min & max of each
     * bucket.
     *
     * @param samples Where to put the samples.
     * @param timestamp_start Where to start sampling from.
     * @param bin_width Width (in time) of each bin.
     * @param num_bins The number of bins.
     * @return std::size_t The actual number of bins generated.
     */
    std::size_t get_samples(TSSample *samples,
                            double timestamp_start,
                            double bin_width,
                            std::size_t num_bins) const override;

    /**
     * @brief Helper function to extract just a single sample.
     *
     * Equivalent to: get_samples(&sample, timestamp, bin_width, 1);
     *
     * @param timestamp Where to start sampling from.
     * @param bin_width Width (in time) of the bin.
     */
    TSSample get_sample(double timestamp, double bin_width) const override;

    /**
     * @brief Returns the timestamps of the oldest and newest samples.
     */
    std::pair<double, double> get_span() const override;

    std::size_t memory_usage() const override;

    std::size_t size() const override;

    /**
     * @brief Adds a new sample to the end of the timeseries. The timestamp of this sample will be
     *
     * @param value
     */
    void push_sample(double value);

  private:
    static int count_trailing_zeros(unsigned long long value);
    static int count_leading_zeros(unsigned long long value);
    std::tuple<double, double, double> _reduce(std::size_t, std::size_t) const;
    
    static constexpr std::size_t CHUNK_SIZE = 16 * 1024;
    mutable std::recursive_mutex _mut;
    std::vector<ChunkedVector<DataStore, CHUNK_SIZE>> _data;
    double _interval;
    double _start;
};
