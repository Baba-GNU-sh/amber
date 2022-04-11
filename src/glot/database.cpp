#include <numeric>
#include "database.hpp"

void Database::register_timeseries(std::string name, std::shared_ptr<TimeSeries> timeseries)
{
    _data[name] = timeseries;
}

const std::map<std::string, std::shared_ptr<TimeSeries>> &Database::data() const
{
    return _data;
}

std::size_t Database::memory_usage() const
{
    std::size_t total_bytes = std::accumulate(_data.begin(),
                                              _data.end(),
                                              static_cast<std::size_t>(0),
                                              [](std::size_t sum, const auto &ts) {
                                                  std::size_t bytes_used =
                                                      ts.second->memory_usage();
                                                  std::size_t total = sum + bytes_used;
                                                  return total;
                                              });
    return total_bytes;
}

std::size_t Database::num_samples() const
{
    return std::accumulate(_data.begin(), _data.end(), 0, [](std::size_t sum, const auto &ts) {
        return sum + ts.second->size();
    });
}
