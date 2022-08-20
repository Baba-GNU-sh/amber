#include "plugin_context.hpp"

PluginContext::PluginContext(Database &database) : m_database(database)
{
    //
}

Database &PluginContext::get_database()
{
    return m_database;
}
