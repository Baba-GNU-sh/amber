#include "timeseries_dense.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

#ifdef _MSC_VER
#include <intrin.h>
#endif

TimeSeriesDense::TimeSeriesDense(double start, double interval)
    : _data(1), _interval(interval), _start(start)
{
}

TimeSeriesDense::TimeSeriesDense(double start, double interval, std::vector<double> init)
    : _data(1), _interval(interval), _start(start)
{
    for (double d : init)
    {
        _data[0].push_back(DataStore{d, d, d});
    }

    int row = 1;
    auto size = init.size() >> 1;
    while (size)
    {
        _data.resize(row + 1);
        for (std::size_t i = 0; i < size; ++i)
        {
            _data[row].push_back(
                DataStore{_data[row - 1][2 * i].sum + _data[row - 1][2 * i + 1].sum,
                          std::min(_data[row - 1][2 * i].min, _data[row - 1][2 * i + 1].min),
                          std::max(_data[row - 1][2 * i].max, _data[row - 1][2 * i + 1].max)});
        }
        size >>= 1;
        ++row;
    }
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
            auto index_first = static_cast<long long>((first - _start) / _interval);
            index_first = std::max(index_first, static_cast<long long>(0));
            auto index_last = static_cast<long long>((last - _start) / _interval);
            index_last = std::min(index_last, static_cast<long long>(_data[0].size()));

            if (index_first == index_last)
            {
                continue;
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

TSSample TimeSeriesDense::get_sample(double timestamp, double bin_width) const
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

/**
 * @brief Fin
 *
 * @param begin
 * @param end
 * @return std::tuple<double, double, double>
 */
std::tuple<double, double, double> TimeSeriesDense::_reduce(std::size_t begin,
                                                            std::size_t end) const
{
    // Data is stored in an array of arrays like so:
    // [1 2 3 4 5 6 7 8]
    // [3 7 11 15]
    // [10 26]
    // [36]
    //
    // The first row contains the raw samples.
    // Each subsequent row contains the sum of two elements in the row above.
    //   I.e. e[N][M] = e[N-1][2M] + e[N-1][2M+1]
    // Where:
    //  - N is the row number
    //  - M is the element in the array
    // Note: the data structure also contains the min and max, but they are left out here for
    // brevity.
    //
    // Each element contains the sum of 2^R raw samples, where R is the index of the row in
    // which it lives. The job of this algorithm is to return the sum, min & max for each element in
    // this list, by visiting the smallest numver of elements possible.

#if defined(__GNUC__) || defined(__GNUG__)
    auto count_trailing_zeros = [](unsigned long long value) {
        return __builtin_ctzll(value | (1ULL << 63));
    };
    auto count_leading_zeros = [](unsigned long long value) {
        return __builtin_clzll(value | 1ULL);
    };
#elif defined _MSC_VER
    auto count_trailing_zeros = [](unsigned long long value) {
        unsigned long leading;
        _BitScanForward64(&leading, value | (1ULL << 63));
        return leading;
    };
    auto count_leading_zeros = [](unsigned long long value) {
        unsigned long leading;
        _BitScanReverse64(&leading, value | 1ULL);
        return 63 ^ leading;
    };
#endif

    // Find the average, min and max for samples between begin and end
    const auto distance = end - begin;
    const auto row_max = static_cast<int>(_data.size() - 1);
    double sum = 0;
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();

    // Run from start to fininsh greedily consuming the highest rows possible
    for (auto iter = begin; iter < end;)
    {
        // Count the number of least significant zeros, which gives us an idea of the largest chunk
        // we are aligned to Note: Passing 0 to __builtin_ctzll() is UB, so set the MSB which will
        // never affect our results
        auto row = count_trailing_zeros(iter);
        row = std::min<unsigned long>(row, row_max);

        // If the remaining number of samples is smaller than one chunk on this row then we would
        // have consumed too much Work out the largest chunk that that will fit in this chunk (by
        // getting log2 of it)
        const auto distance_remaining = end - iter;
        const auto log2_dist = 63 - count_leading_zeros(distance_remaining);
        row = std::min(log2_dist, row);

        const auto index = iter >> row;
        const auto &s = _data[row][index];
        sum += s.sum;
        min = std::min(s.min, min);
        max = std::max(s.max, max);

        iter += (1U << row);
    }

    auto average = sum / distance;
    return std::make_tuple(average, min, max);
}
