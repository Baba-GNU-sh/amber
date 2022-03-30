#pragma once

#include "database.hpp"

class PluginContext
{
  public:
    PluginContext(Database &database) : _database(database)
    {
    }

    Database &get_database()
    {
        return _database;
    }

  private:
    Database &_database;
};
