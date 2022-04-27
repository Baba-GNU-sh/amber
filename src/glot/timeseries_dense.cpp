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
    // TODO find a more efficient solution to concurrent access
    std::lock_guard<std::recursive_mutex> _(_mut);

    // Keep track of which sample we are writing to
    auto *current_sample = samples;

    // The span is important for use later on
    auto span = get_span();

    // Iterate through the bins
    for (std::size_t bin_index = 0; bin_index < num_samples; ++bin_index)
    {
        // Work out the timestamps of the start and end of the bin
        const auto bin_span = std::make_pair<double, double>(
            timestamp_start + bin_width * bin_index, timestamp_start + bin_width * (bin_index + 1));

        const bool bin_is_before_first_sample = bin_span.second < span.first;
        const bool bin_is_after_last_sample = bin_span.first > span.second;
        if (_data[0].empty() || bin_is_before_first_sample || bin_is_after_last_sample)
        {
            // The bin is completely devoid of any samples - don't output anything
            continue;
        }
        else
        {
            current_sample->timestamp = bin_span.first;
            auto index_first = static_cast<long long>((bin_span.first - span.first) / _interval);
            index_first = std::max(index_first, static_cast<long long>(0));
            auto index_last = static_cast<long long>((bin_span.second - span.first) / _interval);
            index_last = std::min(index_last, static_cast<long long>(_data[0].size()));

            if (index_first == index_last)
            {
                const auto value = _data[0][index_first].sum;
                if (current_sample - samples)
                {
                    if ((current_sample - 1)->average == value)
                    {
                        continue;
                    }
                }

                // TODO always return something for the final bin

                current_sample->average = current_sample->min = current_sample->max = value;
                ++current_sample;
            }
            else
            {
                const auto results = _reduce(index_first, index_last);
                current_sample->average = std::get<0>(results);
                current_sample->min = std::get<1>(results);
                current_sample->max = std::get<2>(results);
                ++current_sample;
            }
        }
    }

    const auto count = current_sample - samples;
    return count;
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

std::size_t TimeSeriesDense::memory_usage() const
{
    std::size_t size = _data.size() * sizeof(ChunkedVector<DataStore, CHUNK_SIZE>);
    std::size_t total_bytes =
        std::accumulate(_data.begin(), _data.end(), size, [](std::size_t total, const auto &row) {
            const std::size_t capacity = row.capacity();
            return total + (capacity * sizeof(row[0]));
        });
    return total_bytes;
}

std::size_t TimeSeriesDense::size() const
{
    return _data[0].size();
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

#if defined(__GNUC__) || defined(__GNUG__)
int TimeSeriesDense::count_trailing_zeros(unsigned long long value)
{
    return __builtin_ctzll(value | (1ULL << 63));
};

int TimeSeriesDense::count_leading_zeros(unsigned long long value)
{
    return __builtin_clzll(value | 1ULL);
};

#elif defined _MSC_VER
int TimeSeriesDense::count_trailing_zeros(unsigned long long value)
{
    unsigned long leading;
    _BitScanForward64(&leading, value | (1ULL << 63));
    return leading;
};

int TimeSeriesDense::count_leading_zeros(unsigned long long value)
{
    unsigned long leading;
    _BitScanReverse64(&leading, value | 1ULL);
    return 63 ^ leading;
};

#endif

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
        // we are aligned to
        auto row = count_trailing_zeros(iter);
        row = std::min(row, row_max);

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
