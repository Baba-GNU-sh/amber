#pragma once

#include <memory>
#include <vector>

#include "audiofile_plugin.hpp"
#include "plugin.hpp"

class PluginManager
{
  public:
    PluginManager(PluginContext &context)
    {
        _plugins.push_back(std::make_shared<AudioFilePlugin>(context, "audio/CantinaBand60.wav"));
    }

  private:
    std::vector<std::shared_ptr<Plugin>> _plugins;
};
