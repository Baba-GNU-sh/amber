/**
 * @file Main include for tsdb.
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <memory>
#include <stdexcept>

using Time = uint64_t;

struct Sample
{
	Time time;
	double value;
};

using TimeSeries = std::vector<Sample>;

class TimeSeriesBase
{
public:
	virtual ~TimeSeriesBase() = default;

	/**
	 * @brief Get the timestamp of the first and last samples in the dataset.
	 * 
	 * @return std::pair<Time, Time> 
	 */
	virtual std::pair<Time, Time> span() = 0;

	/**
	 * @brief Get a value of the dataset at a given time.
	 * 
	 * @param time 
	 * @return double 
	 */
	virtual double get(const Time &time) = 0;

	/**
	 * @brief Get the mean of a range in the dataset. Returns NaN if there are no samples within this range.
	 * 
	 * @param start 
	 * @param end 
	 * @return double 
	 */
	virtual double mean(const Time &start, const Time &end) = 0;

	/**
	 * @brief Get the max value within a time range.
	 * 
	 * @param start 
	 * @param end 
	 * @return double 
	 */
	virtual double max(const Time &start, const Time &end) = 0;

	/**
	 * @brief Get the minimum value within a time range.
	 * 
	 * @param start 
	 * @param end 
	 * @return double 
	 */
	virtual double min(const Time &start, const Time &end) = 0;
};

class SparseTimeSeries : public TimeSeriesBase
{
public:
	SparseTimeSeries(bool sorted = false) : m_sorted(sorted) {}

	void push(const Time &time, double value)
	{
		push({time, value});
	}

	void push(const Sample &sample)
	{
		// TODO find where to insert by doing a binary search on the time stamp
		time_series.push_back(sample);

		if (!m_sorted)
		{
			std::sort(
				time_series.begin(),
				time_series.end(),
				[](const Sample &a, const Sample &b)
				{
					return a.time < b.time;
				});
		}
	}

	void push_some(const std::vector<Sample> &samples)
	{
		time_series.insert(time_series.end(), samples.begin(), samples.end());

		if (!m_sorted)
		{
			std::sort(
				time_series.begin(),
				time_series.end(),
				[](const Sample &a, const Sample &b)
				{
					return a.time < b.time;
				});
		}
	}

	std::pair<Time, Time> span() override
	{
		return std::make_pair(time_series.front().time, time_series.back().time);
	}

	double get(const Time &time) override
	{
		auto span = this->span();

		if (time < span.first)
		{
			throw std::runtime_error("Time is before the first sample");
		}
		else if (time > span.second)
		{
			throw std::runtime_error("Time is after the last sample");
		}

		auto iter = std::upper_bound(time_series.begin(),
									 time_series.end(),
									 time,
									 [](Time value, const Sample &sample)
									 {
										 return sample.time >= value;
									 });

		if (iter == time_series.end())
		{
			// This should never happen, due to the check above
			throw std::runtime_error("Time is larger than the bounds of the system");
		}
		else
		{
			auto &sample = *iter;
			if (sample.time == time)
				return iter->value;
			else
			{
				// Interpolate!
				auto prev_iter = iter - 1;
				auto &prev_sample = *prev_iter;
				return lerp(sample, prev_sample, time);
			}
		}
	}

	std::pair<std::vector<Sample>::iterator, std::vector<Sample>::iterator> range(const Time &start, const Time &end)
	{
		auto span = this->span();

		if (end < span.first)
		{
			return {};
		}
		else if (start > span.second)
		{
			return {};
		}
		else
		{
			// Find the first matching sample
			auto first = std::upper_bound(time_series.begin(),
										  time_series.end(),
										  start,
										  [](Time value, const Sample &sample)
										  {
											  return sample.time >= value;
										  });

			// Find the last matching sample
			auto last = std::upper_bound(time_series.begin(),
										 time_series.end(),
										 end,
										 [](Time value, const Sample &sample)
										 {
											 return sample.time > value;
										 });

			return std::make_pair(first, last);
		}
	}

	double mean(const Time &start, const Time &end) override
	{
		auto r = range(start, end);
		auto sum = std::accumulate(r.first, r.second, 0.0, [](double a, const Sample &b)
								   { return a + b.value; });
		return sum / std::distance(r.first, r.second);
	}

	std::vector<double> mean(const Time &start, const Time &step, std::size_t count)
	{
		std::vector<double> ret;
		for (int i = 0; i < count; ++i)
		{
			ret.push_back(mean(i * step, (i + 1) * step));
		}

		return ret;
	}

	double max(const Time &start, const Time &end) override
	{
		auto r = range(start, end);
		auto max = std::max_element(r.first, r.second, [](const Sample &a, const Sample &b)
									{ return a.value < b.value; });
		return max->value;
	}

	double min(const Time &start, const Time &end) override
	{
		auto r = range(start, end);
		auto min = std::min_element(r.first, r.second, [](const Sample &a, const Sample &b)
									{ return a.value < b.value; });
		return min->value;
	}

private:
	static double lerp(const Sample &a, const Sample &b, const Time &time)
	{
		auto delta = a.time - b.time;
		auto alpha = static_cast<double>(time - b.time) / delta;
		return ((1 - alpha) * b.value) + (alpha * a.value);
	}

private:
	const bool m_sorted;
	std::vector<Sample> time_series;
};

class TimeSeriesDB
{
	void add(const std::string &name, const std::shared_ptr<TimeSeriesBase> ts)
	{
		auto iter = m_data.find(name);
		if (iter != m_data.end())
		{
			throw std::runtime_error("Time series with this name already exists");
		}
		m_data[name] = ts;
	}

	void erase(const std::string &name)
	{
		auto iter = m_data.find(name);
		if (iter == m_data.end())
		{
			throw std::runtime_error("Time series with this name does not exist");
		}

		m_data.erase(iter);
	}

	std::map<std::string, std::shared_ptr<TimeSeriesBase>> m_data;
};
