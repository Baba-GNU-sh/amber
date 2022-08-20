#pragma once

#include "plugin.hpp"
#include "plugin_context.hpp"
#include <database/timeseries.hpp>
#include <database/timeseries_dense.hpp>
#include <AudioFile.h>
#include <spdlog/logger.h>
#include <thread>

class AudioFilePlugin : public Plugin
{
  public:
    AudioFilePlugin(PluginContext &pluggy, std::string_view filename);
    virtual ~AudioFilePlugin();

    void start() override;
    void stop() override;
    bool is_running() const override;
    void draw_menu() override;

  private:
    void background_worker();

    AudioFile<float> m_audioFile;
    std::thread m_thread;
    std::atomic<bool> m_running;
    std::shared_ptr<TimeSeriesDense> m_ts;
    std::shared_ptr<spdlog::logger> m_logger;
    std::size_t m_current_sample;
    std::string m_filename;
};
