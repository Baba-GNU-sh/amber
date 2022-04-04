#pragma once

#include <mutex>
#include <tuple>
#include <vector>

#include "timeseries.hpp"

/**
 * @brief A densely packed time series with a fixed data rate.
 * This time series is useful if you
 */
class TimeSeriesDense : public TimeSeries
{
  public:
    TimeSeriesDense(double start, double interval);

    std::size_t get_samples(TSSample *samples,
                            double timestamp_start,
                            double bin_width,
                            std::size_t num_samples) const override;
    TSSample get_sample(double timestamp, double bin_width) override;
    std::pair<double, double> get_span() const override;
    void push_sample(double value);
    void push_samples(const std::vector<double> &value);

  private:
    std::tuple<double, double, double> _reduce(std::vector<double>::const_iterator,
                                               std::vector<double>::const_iterator) const;
    mutable std::recursive_mutex _mut;
    std::vector<double> _data;
    double _interval;
    double _start;
};
