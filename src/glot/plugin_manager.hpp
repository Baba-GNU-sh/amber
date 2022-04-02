#pragma once

#include <memory>
#include <vector>

#include "plugin.hpp"
#include "plugin_context.hpp"

struct PluginInfo
{
    std::string name;
    bool show_menu;
    std::shared_ptr<Plugin> plugin;
};

class PluginManager
{
  public:
    PluginManager(PluginContext &context);
    void start_all();
    void stop_all();
    void draw_menu();
    void draw_dialogs() const;

  private:
    std::vector<PluginInfo> _plugins;
};
