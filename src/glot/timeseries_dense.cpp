#include "timeseries_dense.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

TimeSeriesDense::TimeSeriesDense(double start, double interval) : _interval(interval), _start(start)
{
}

std::size_t TimeSeriesDense::get_samples(TSSample *samples,
                                         double timestamp_start,
                                         double bin_width,
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
        if (_data.empty() || (first < span.first && last < span.first) || (first > span.second && last > span.second))
        {
            sample.timestamp = first;
            sample.average = sample.min = sample.max = std::numeric_limits<double>::quiet_NaN();
        }
        else
        {
            sample.timestamp = first;

            auto index_first = static_cast<std::size_t>((first - _start) / _interval);
            index_first = std::max(index_first, 0UL);
            auto index_last = static_cast<std::size_t>((last - _start) / _interval);
            index_last = std::min(index_last, _data.size());

            auto iter_first = _data.begin() + index_first;
            auto iter_last = _data.begin() + index_last;

            if (index_first == index_last)
            {
                sample.average = _data[index_first];
                sample.min = sample.average;
                sample.max = sample.average;
            }
            else
            {
                const auto results = _reduce(iter_first, iter_last);
                sample.average = std::get<0>(results);
                sample.min = std::get<1>(results);
                sample.max = std::get<2>(results);
            }
        }

        index++;
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

void TimeSeriesDense::push_sample(double value)
{
    std::lock_guard<std::recursive_mutex> _(_mut);
    _data.push_back(value);
}

void TimeSeriesDense::push_samples(const std::vector<double> &data)
{
    std::lock_guard<std::recursive_mutex> _(_mut);
    _data.insert(_data.end(), data.begin(), data.end());
}

std::tuple<double, double, double> TimeSeriesDense::_reduce(
    std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end) const
{
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    double sum = 0.0;
    for (auto iter = begin; iter != end; iter++)
    {
        min = std::min(min, *iter);
        max = std::max(max, *iter);
        sum += *iter;
    }
    const auto average = sum / (end - begin);
    return std::make_tuple(average, min, max);
}

// std::tuple<double, double, double> TimeSeriesDense::_reduce(
//     std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end) const
// {
//     const auto len = (end - begin);
//     auto average = std::reduce(std::execution::seq, begin, end, 0.0) / len;
//     auto minmax = std::minmax_element(std::execution::seq, begin, end);
//     return std::make_tuple(average, *minmax.first, *minmax.second);
// }
