#include "plugin_context.hpp"

PluginContext::PluginContext(Database &database) : _database(database)
{
    //
}

Database &PluginContext::get_database()
{
    return _database;
}
