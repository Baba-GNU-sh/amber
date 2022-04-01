#pragma once

#include <memory>
#include <vector>

#include "plugin.hpp"

#include "audiofile_plugin.hpp"

class PluginManager
{
  public:
    PluginManager(PluginContext &context);

  private:
    std::vector<std::shared_ptr<Plugin>> _plugins;
};
