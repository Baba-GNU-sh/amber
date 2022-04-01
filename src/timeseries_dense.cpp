#include "timeseries_dense.hpp"

TimeSeriesDense::TimeSeriesDense(double start, double interval) : _interval(interval), _start(start)
{
    //
}

std::size_t TimeSeriesDense::get_samples(TSSample *samples, double timestamp_start, double bin_width,
                                         std::size_t num_samples) const
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
            {
                sample.average = _data[index_first];
                sample.min = sample.average;
                sample.max = sample.average;
            }
            else
            {
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
        }

        index++;

        // // For now, just find the nearest sample to first timestamp
        // auto start = timestamp_start - _start;
        // if (start < )
    }

    return index;
}

TSSample TimeSeriesDense::get_sample(double timestamp, double bin_width)
{
    TSSample sample;
    get_samples(&sample, timestamp, bin_width, 1);
    return sample;
}

std::pair<double, double> TimeSeriesDense::get_span() const
{
    std::lock_guard<std::recursive_mutex> _(_mut);
    double last = _start + _data.size() * _interval;
    return std::pair(_start, last);
}

/**
 * @brief Adds a new sample to the timeseries.
 * @param value The value of the sample to add.
 */
void TimeSeriesDense::push_samples(double value)
{
    std::lock_guard<std::recursive_mutex> _(_mut);
    _data.push_back(value);
}
