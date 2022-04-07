#include "timeseries_dense.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

TimeSeriesDense::TimeSeriesDense(double start, double interval)
    : _data(1), _interval(interval), _start(start)
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
        if (_data[0].empty() || (first < span.first && last < span.first) ||
            (first > span.second && last > span.second))
        {
            sample.timestamp = first;
            sample.average = sample.min = sample.max = std::numeric_limits<double>::quiet_NaN();
        }
        else
        {
            sample.timestamp = first;

            auto index_first = static_cast<ssize_t>((first - _start) / _interval);
            index_first = std::max(index_first, static_cast<ssize_t>(0));
            auto index_last = static_cast<ssize_t>((last - _start) / _interval);
            index_last = std::min(index_last, static_cast<ssize_t>(_data[0].size()));

            if (index_first == index_last)
            {
                const auto &s = _data[0][index_first];
                sample.average = s.sum;
                sample.min = s.min;
                sample.max = s.max;
            }
            else
            {
                const auto results = _reduce(index_first, index_last);
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
    const double last = _start + (_data[0].size() * _interval);
    return std::make_pair(_start, last);
}

void TimeSeriesDense::push_sample(double value)
{
    std::lock_guard<std::recursive_mutex> _(_mut);
    _data[0].push_back(DataStore{value, value, value});
    std::size_t sz = _data[0].size();
    std::size_t index = 0;
    while (sz /= 2)
    {
        ++index;
        if (index >= _data.size())
        {
            _data.resize(index + 1);
        }

        const auto &prev = _data[index - 1];
        auto &buf = _data[index];
        if (sz > buf.size())
        {
            const auto last = prev[prev.size() - 1];
            const auto second_last = prev[prev.size() - 2];

            const auto sum = last.sum + second_last.sum;
            const auto min = std::min(last.min, second_last.min);
            const auto max = std::max(last.max, second_last.max);
            buf.push_back(DataStore{sum, min, max});
        }
    }
}

std::tuple<double, double, double> TimeSeriesDense::_reduce(std::size_t begin,
                                                            std::size_t end) const
{
    const auto distance = end - begin;
    const auto row_max = static_cast<int>(_data.size() - 1);
    double sum = 0;
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();

    while (begin < end)
    {
        auto row = __builtin_ctzll(begin);
        row = std::min(row, row_max);

        const auto distance_remaining = end - begin;
        const auto log2_dist = 63 - __builtin_clzll(distance_remaining);
        row = std::min(log2_dist, row);

        const auto index = begin >> row;
        const auto &s = _data[row][index];
        sum += s.sum;
        min = std::min(s.min, min);
        max = std::max(s.max, max);

        begin += (1U << row);
    }

    auto average = sum / distance;
    return std::make_tuple(average, min, max);
}
