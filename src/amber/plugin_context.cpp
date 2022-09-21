#include "plugin_context.hpp"

using namespace amber;

PluginContext::PluginContext(database::Database &database) : m_database(database)
{
    //
}

database::Database &PluginContext::get_database()
{
    return m_database;
}
