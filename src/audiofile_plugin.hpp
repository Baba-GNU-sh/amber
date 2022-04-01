#pragma once

#include "plugin_context.hpp"
#include "timeseries.hpp"
#include "timeseries_dense.hpp"
#include <AudioFile.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>
#include "plugin.hpp"

class AudioFilePlugin : public Plugin
{
  public:
    AudioFilePlugin(PluginContext &pluggy, std::string_view filename);
    ~AudioFilePlugin();
    void thread();
    float *data();
    std::size_t size();
    int sample_rate() const;

  private:
    AudioFile<float> _audioFile;
    std::thread _thread;
    std::atomic<bool> _cancel_flag;
    std::shared_ptr<TimeSeriesDense> _ts;
    std::shared_ptr<spdlog::logger> _logger;
    std::size_t _current_sample;
};
