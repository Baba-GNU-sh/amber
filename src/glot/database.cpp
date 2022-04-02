#include "database.hpp"

void Database::register_timeseries(std::string name, std::shared_ptr<TimeSeries> timeseries)
{
    _data[name] = timeseries;
}

const std::map<std::string, std::shared_ptr<TimeSeries>> &Database::data() const
{
    return _data;
}
