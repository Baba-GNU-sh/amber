#pragma once

#include <mutex>
#include "timeseries.hpp"
#include "chunked_vector.hpp"

struct Sample
{
    double time;
    double value;
};

class TimeSeriesSparse : public TimeSeries
{
  public:
    std::size_t get_samples(TSSample *samples,
                            double timestamp_start,
                            double bin_width,
                            std::size_t num_bins) const override;
    TSSample get_sample(double timestamp, double bin_width) const override;
    std::pair<double, double> get_span() const override;
    std::size_t memory_usage() const override;
    std::size_t size() const override;
    void push_sample(const Sample &value);

  private:
    static constexpr std::size_t CHUNK_SIZE = 10 * 1024;
    ChunkedVector<Sample, CHUNK_SIZE> m_data;
};
