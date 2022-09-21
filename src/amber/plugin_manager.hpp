#pragma once

#include <memory>
#include <vector>

#include "plugin.hpp"
#include "plugin_context.hpp"

namespace amber
{
class PluginManager
{
    struct PluginInfo
    {
        std::string name;
        bool show_menu;
        std::shared_ptr<Plugin> plugin;
    };

  public:
    PluginManager();
    void add_plugin(std::string_view name, std::shared_ptr<Plugin> plugin);
    void start_all();
    void stop_all();
    void draw_menu();
    void draw_dialogs() const;

  private:
    std::vector<PluginInfo> m_plugins;
};
} // namespace amber
