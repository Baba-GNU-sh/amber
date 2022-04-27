#pragma once

#include <spdlog/spdlog.h>
#include <thread>

#include "plugin.hpp"
#include "plugin_context.hpp"
#include <database/timeseries_dense.hpp>

enum WaveType
{
    Sine = 0,
    Square = 1,
    Triangle = 2,
    SawTooth = 3
};

struct WaveSettings
{
    WaveType type;
    float frequency;
    float amplitude;
};

class WaveGenPlugin : public Plugin
{
  public:
    WaveGenPlugin(PluginContext &ctx);
    virtual ~WaveGenPlugin();

    void start() override;
    void stop() override;
    bool is_running() const override;
    void draw_menu() override;

  private:
    void thread_handler();
    double sample_value(WaveType settings, double time) const;
    PluginContext &_ctx;
    const unsigned int _sample_rate = 10'000;
    std::shared_ptr<spdlog::logger> _logger;
    std::atomic<bool> _running = false;
    std::thread _thread;
    std::shared_ptr<TimeSeriesDense> _ts;
    std::size_t _ticks = 0;
    mutable std::mutex _mutex;
    WaveSettings _settings;
};
