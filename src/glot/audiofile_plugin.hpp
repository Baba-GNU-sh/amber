#pragma once

#include "plugin.hpp"
#include "plugin_context.hpp"
#include "timeseries.hpp"
#include "timeseries_dense.hpp"
#include <AudioFile.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>

class AudioFilePlugin : public Plugin
{
  public:
    AudioFilePlugin(PluginContext &pluggy, std::string_view filename);
    ~AudioFilePlugin();
    void thread();
    float *data();
    std::size_t size();
    int sample_rate() const;

    void start();
    void stop();
    void draw_menu();

  private:
    AudioFile<float> _audioFile;
    std::thread _thread;
    std::atomic<bool> _running;
    std::shared_ptr<TimeSeriesDense> _ts;
    std::shared_ptr<spdlog::logger> _logger;
    std::size_t _current_sample;
    std::string _filename;
};
