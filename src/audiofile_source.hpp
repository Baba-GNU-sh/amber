#pragma once

#include "plugin_context.hpp"
#include "timeseries.hpp"
#include "timeseries_dense.hpp"
#include <AudioFile.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>

class AudioFileSource
{
  public:
    AudioFileSource(PluginContext &pluggy, std::string_view filename) : _cancel_flag(false), _current_sample(0)
    {
        _audioFile.load(std::string(filename));
        _audioFile.printSummary();

        auto ts = std::make_shared<TimeSeriesDense>(0.0, 1.0 / _audioFile.getSampleRate());
        pluggy.get_database().register_timeseries(std::string(filename), ts);

        _ts = ts;

        _thread = std::thread(&AudioFileSource::thread, this);

        _logger = spdlog::stdout_color_mt("AudioFileSource");
        _logger->info("Hello from audio plugin!");
    }

    ~AudioFileSource()
    {
        _cancel_flag = true;
        _thread.join();
        _logger->info("Goodbye from audio plugin!");
    }

    void thread()
    {
        using namespace std::chrono_literals;

        while (!_cancel_flag)
        {
            std::this_thread::sleep_for(1ms);

            auto sample_count = _audioFile.getSampleRate() / 1000.0;
            for (int i = 0; i < sample_count; ++i)
            {
                if (_current_sample >= _audioFile.getNumSamplesPerChannel())
                {
                    _current_sample = 0;
                }

                auto sample = _audioFile.samples[0][_current_sample];
                _ts->push_samples(sample);

                _current_sample++;
            }

            auto span = _ts->get_span();
        }
    }

    float *data()
    {
        return &_audioFile.samples[0][0];
    }

    std::size_t size()
    {
        return _audioFile.getNumSamplesPerChannel();
    }

    int sample_rate() const
    {
        return _audioFile.getSampleRate();
    }

  private:
    AudioFile<float> _audioFile;
    std::thread _thread;
    std::atomic<bool> _cancel_flag;
    std::shared_ptr<TimeSeriesDense> _ts;
    std::shared_ptr<spdlog::logger> _logger;
    std::size_t _current_sample;
};
