#pragma once

#include "timeseries.hpp"
#include <map>
#include <memory>
#include <string>

class Database
{
  public:
    /**
     * @brief Add a new time series to the database.
     *
     * @param name The name of the timeseries.
     * @param timeseries The timeseries to add.
     */
    void register_timeseries(std::string name, std::shared_ptr<TimeSeries> timeseries);

    /**
     * @brief Get immutable access to all the entire list of time series in the
     * database.
     *
     * @return const std::map<std::string, std::shared_ptr<TimeSeries>>&
     */
    const std::map<std::string, std::shared_ptr<TimeSeries>> &data() const;

    std::size_t memory_usage() const;

    std::size_t num_samples() const;

  private:
    std::map<std::string, std::shared_ptr<TimeSeries>> _data;
};
