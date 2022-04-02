#pragma once

#include <thread>
#include <spdlog/spdlog.h>

#include "plugin.hpp"
#include "plugin_context.hpp"
#include "timeseries_dense.hpp"

enum class WaveType
{
    Sine
};

class WaveGenPlugin : public Plugin
{
  public:
    WaveGenPlugin(PluginContext &ctx);
    ~WaveGenPlugin();
    void start() override;
    void stop() override;
    bool is_running() const override;
    void draw_menu() override;

  private:
    void thread_handler();
    PluginContext &_ctx;
    float _frequency = 1.0f;
    const unsigned int _sample_rate = 1000;
    WaveType _wave_type = WaveType::Sine;
    std::shared_ptr<spdlog::logger> _logger;
    std::atomic<bool> _running = false;
    std::thread _thread;
    std::shared_ptr<TimeSeriesDense> _ts;
    std::size_t _ticks = 0;
};
