#pragma once

#include <spdlog/spdlog.h>
#include <thread>

#include "plugin.hpp"
#include "plugin_context.hpp"
#include "timeseries_sparse.hpp"

class RandomDataPlugin : public Plugin
{
  public:
    RandomDataPlugin(PluginContext &ctx);
    virtual ~RandomDataPlugin();

    void start() override;
    void stop() override;
    bool is_running() const override;
    void draw_menu() override;

  private:
    void thread_handler();
    PluginContext &_ctx;
    std::shared_ptr<spdlog::logger> _logger;
    std::atomic<bool> _running = false;
    std::thread _thread;
    std::shared_ptr<TimeSeriesSparse> _ts;
    mutable std::mutex _mutex;
};
