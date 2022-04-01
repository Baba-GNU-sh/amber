#pragma once

#include <mutex>
#include <vector>

#include "timeseries.hpp"

/**
 * @brief Defines the interface which all time series must implement.
 */
class TimeSeriesDense : public TimeSeries
{
  public:
    TimeSeriesDense(double start, double interval);

    std::size_t get_samples(TSSample *samples,
                            double timestamp_start,
                            double bin_width,
                            std::size_t num_samples) const override;

    TSSample get_sample(double timestamp, double bin_width);
    std::pair<double, double> get_span() const override;

    /**
     * @brief Adds a new sample to the timeseries.
     * @param value The value of the sample to add.
     */
    void push_samples(double value);

    mutable std::recursive_mutex _mut;
    std::vector<double> _data;
    double _interval;
    double _start;
};
