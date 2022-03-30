#pragma once

class PluginManager
{
  public:
    PluginManager()
    {
    }

  private:
    std::vector<std::shared_ptr<Plugin>> _plugins;
};
