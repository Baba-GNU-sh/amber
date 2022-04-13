#include "timeseries_sparse.hpp"
#include <cassert>
#include <algorithm>

std::size_t TimeSeriesSparse::get_samples(TSSample *samples,
                                          double timestamp_start,
                                          double bin_width,
                                          std::size_t num_bins) const
{
    if (m_data.empty())
    {
        return 0;
    }

    int j = 0;
    for (std::size_t i = 0; i < num_bins; ++i)
    {
        const auto ts = timestamp_start + bin_width * i;
        const auto sample = get_sample(ts, bin_width);
        if (j > 0 && sample.timestamp == samples[j-1].timestamp)
        {
            continue;
        }
        samples[j++] = sample;
    }
    return j;
}

TSSample TimeSeriesSparse::get_sample(double timestamp, double bin_width) const
{
    (void)bin_width;
    auto lower_bound = std::lower_bound(
        m_data.begin(), m_data.end(), Sample{timestamp, 0.0}, [](const auto &r1, const auto &r2) {
            return r1.time < r2.time;
        });
    auto time = static_cast<float>(lower_bound->time);
    auto value = static_cast<float>(lower_bound->value);
    return TSSample{time, value, value, value};
}

std::pair<double, double> TimeSeriesSparse::get_span() const
{
    if (m_data.empty())
    {
        return std::make_pair(0.0, 0.0);
    }
    else
    {
        return std::make_pair(m_data.front().time, m_data.back().time);
    }
}

std::size_t TimeSeriesSparse::memory_usage() const
{
    return m_data.size() * sizeof(m_data[0]);
}

std::size_t TimeSeriesSparse::size() const
{
    return m_data.size();
}

void TimeSeriesSparse::push_sample(const Sample &value)
{
    assert(value.time > get_span().second);
    m_data.push_back(value);
}