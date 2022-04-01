#include "plugin_manager.hpp"

PluginManager::PluginManager(PluginContext &context)
{
    _plugins.push_back(std::make_shared<AudioFilePlugin>(context, "audio/CantinaBand60.wav"));
}
