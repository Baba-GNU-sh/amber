#include "plugin_manager.hpp"
#include <imgui.h>

using namespace amber;

PluginManager::PluginManager()
{
    //
}

void PluginManager::add_plugin(std::string_view name, std::shared_ptr<Plugin> plugin)
{
    m_plugins.push_back(PluginInfo{std::string(name), false, plugin});
}

void PluginManager::start_all()
{
    for (auto plugin : m_plugins)
    {
        plugin.plugin->start();
    }
}

void PluginManager::stop_all()
{
    for (auto plugin : m_plugins)
    {
        plugin.plugin->stop();
    }
}

void PluginManager::draw_menu()
{
    for (auto &plugin : m_plugins)
    {
        if (ImGui::BeginMenu(plugin.name.c_str()))
        {
            if (plugin.plugin->is_running())
            {
                if (ImGui::MenuItem("Stop"))
                {
                    plugin.plugin->stop();
                }
            }
            else
            {
                if (ImGui::MenuItem("Start"))
                {
                    plugin.plugin->start();
                }
            }

            if (ImGui::MenuItem("Show Display"))
            {
                plugin.show_menu = !plugin.show_menu;
            }

            ImGui::EndMenu();
        }
    }
}

void PluginManager::draw_dialogs() const
{
    for (auto plugin : m_plugins)
    {
        if (plugin.show_menu)
        {
            plugin.plugin->draw_menu();
        }
    }
}
