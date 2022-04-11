#include "audiofile_plugin.hpp"

#include <imgui.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdexcept>

AudioFilePlugin::AudioFilePlugin(PluginContext &pluggy, std::string_view filename)
    : _running(false), _current_sample(0), _filename(filename)
{
    if (!_audioFile.load(std::string(filename)))
    {
        throw std::runtime_error("Unable to load audio file");
    }

    auto ts = std::make_shared<TimeSeriesDense>(0.0, 1.0 / _audioFile.getSampleRate());
    pluggy.get_database().register_timeseries(std::string(filename), ts);
    _ts = ts;

    _logger = spdlog::stdout_color_mt("AudioFilePlugin");

    _logger->info("Loaded audio file {}", filename);
    _logger->info("Sample rate: {}", _audioFile.getSampleRate());
    _logger->info("Total samples: {}", _audioFile.getNumSamplesPerChannel());
    _logger->info("Initialized");
}

AudioFilePlugin::~AudioFilePlugin()
{
    if (_running)
    {
        _running = false;
        _thread.join();
    }
    _logger->info("Goodbye!");
}

void AudioFilePlugin::thread()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    const auto sample_period = std::chrono::duration<double>(1s) / _audioFile.getSampleRate();
    auto prevtime = std::chrono::steady_clock::now();

    while (_running)
    {
        std::this_thread::sleep_for(10ms);

        const auto now = steady_clock::now();
        auto delta = std::chrono::duration<double>(now - prevtime);
        prevtime = now;

        while (delta > seconds(0))
        {
            if (_current_sample >= static_cast<std::size_t>(_audioFile.getNumSamplesPerChannel()))
            {
                _current_sample = 0;
            }

            auto sample = _audioFile.samples[0][_current_sample++];
            _ts->push_sample(sample);

            delta -= sample_period;
        }
    }
}

float *AudioFilePlugin::data()
{
    return &_audioFile.samples[0][0];
}

std::size_t AudioFilePlugin::size()
{
    return _audioFile.getNumSamplesPerChannel();
}

int AudioFilePlugin::sample_rate() const
{
    return _audioFile.getSampleRate();
}

void AudioFilePlugin::start()
{
    if (!_running)
    {
        _running = true;
        _thread = std::thread(&AudioFilePlugin::thread, this);
        _logger->info("Started");
    }
}

void AudioFilePlugin::stop()
{
    if (_running)
    {
        _running = false;
        _thread.join();
        _logger->info("Stopped");
    }
}

bool AudioFilePlugin::is_running() const
{
    return _running;
}

void AudioFilePlugin::draw_menu()
{
    ImGui::Begin("AudioFile");
    ImGui::Text("File: %s", _filename.c_str());
    ImGui::Text("Sample Rate: %u", _audioFile.getSampleRate());
}
