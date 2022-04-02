#include "plugin_manager.hpp"
#include <imgui.h>

#include "audiofile_plugin.hpp"
#include "wavegen_plugin.hpp"

PluginManager::PluginManager(PluginContext &context)
{
    _plugins.push_back({"audiofile",
                        false,
                        std::make_shared<AudioFilePlugin>(context, "audio/CantinaBand60.wav")});
    _plugins.push_back({"wavegen", false, std::make_shared<WaveGenPlugin>(context)});
}

void PluginManager::start_all()
{
    for (auto plugin : _plugins)
    {
        plugin.plugin->start();
    }
}

void PluginManager::stop_all()
{
    for (auto plugin : _plugins)
    {
        plugin.plugin->stop();
    }
}

void PluginManager::draw_menu()
{
    for (auto &plugin : _plugins)
    {
        if (ImGui::BeginMenu(plugin.name.c_str()))
        {
            if (ImGui::MenuItem("Start"))
            {
                plugin.plugin->start();
            }

            if (ImGui::MenuItem("Stop"))
            {
                plugin.plugin->stop();
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
    for (auto plugin : _plugins)
    {
        if (plugin.show_menu)
        {
            plugin.plugin->draw_menu();
        }
    }
}
