#pragma once

#include <spdlog/spdlog.h>
#include <thread>

#include "plugin.hpp"
#include "plugin_context.hpp"
#include <database/timeseries_dense.hpp>

namespace amber
{
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
    PluginContext &m_ctx;
    const unsigned int m_sample_rate = 10'000;
    std::shared_ptr<spdlog::logger> m_logger;
    std::atomic<bool> m_running = false;
    std::thread m_thread;
    std::shared_ptr<database::TimeSeriesDense> m_ts;
    mutable std::mutex m_mutex;
    WaveSettings m_settings;
};
} // namespace amber
