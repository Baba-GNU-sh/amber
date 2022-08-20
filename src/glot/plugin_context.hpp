#pragma once

#include <database/database.hpp>

class PluginContext
{
  public:
    PluginContext(Database &database);
    Database &get_database();

  private:
    Database &m_database;
};
