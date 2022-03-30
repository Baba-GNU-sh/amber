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
    TimeSeriesDense(double start, double interval) : _interval(interval), _start(start)
    {
        //
    }

    std::size_t get_samples(TSSample *samples, double timestamp_start, double bin_width,
                     std::size_t num_samples) const override
    {
        std::lock_guard<std::recursive_mutex> _(_mut);
        std::size_t index = 0;
        for (std::size_t i = 0; i < num_samples; ++i)
        {
            double first = timestamp_start + bin_width * i;
            double last = timestamp_start + bin_width * (i + 1);

            TSSample &sample = samples[index];

            auto span = get_span();
            if (first < span.first || first >= span.second || last < span.first || last >= span.second)
            {
                sample.timestamp = first;
                sample.average = std::numeric_limits<double>::quiet_NaN();
                sample.min = sample.average;
                sample.max = sample.average;
            }
            else
            {
                sample.timestamp = first;

                auto index_first = static_cast<int>((first - _start) / _interval);
                auto index_last = static_cast<int>((last - _start) / _interval);

                if (index_first == index_last)
                    continue;

                double average = 0.0;
                double max = -1000000;
                double min = 1000000;
                for (int i = index_first; i < index_last; i++)
                {
                    average += _data[i];
                    max = std::max(max, _data[i]);
                    min = std::min(min, _data[i]);
                }
                average /= (index_last - index_first);
                sample.average = average;
                sample.min = min;
                sample.max = max;
            }

            index++;

            // // For now, just find the nearest sample to first timestamp
            // auto start = timestamp_start - _start;
            // if (start < )
        }

        return index;
    }

    std::pair<double, double> get_span() const override
    {
        std::lock_guard<std::recursive_mutex> _(_mut);
        double last = _start + _data.size() * _interval;
        return std::pair(_start, last);
    }

    /**
     * @brief Adds a new sample to the timeseries.
     * @param value The value of the sample to add.
     */
    void push_samples(double value)
    {
        std::lock_guard<std::recursive_mutex> _(_mut);
        _data.push_back(value);
    }

    mutable std::recursive_mutex _mut;
    std::vector<double> _data;
    double _interval;
    double _start;
};
