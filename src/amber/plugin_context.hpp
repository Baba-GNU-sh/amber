#pragma once

#include <database/database.hpp>

namespace amber
{
class PluginContext
{
  public:
    PluginContext(database::Database &database);
    database::Database &get_database();

  private:
    database::Database &m_database;
};
} // namespace amber
