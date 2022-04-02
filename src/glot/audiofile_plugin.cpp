#include "audiofile_plugin.hpp"
#include <imgui.h>

AudioFilePlugin::AudioFilePlugin(PluginContext &pluggy, std::string_view filename)
    : _running(false), _current_sample(0), _filename(filename)
{
    _audioFile.load(std::string(filename));
    _audioFile.printSummary();

    auto ts = std::make_shared<TimeSeriesDense>(0.0, 1.0 / _audioFile.getSampleRate());
    pluggy.get_database().register_timeseries(std::string(filename), ts);

    _ts = ts;

    _logger = spdlog::stdout_color_mt("AudioFilePlugin");
    _logger->info("Plugin initialized");
}

AudioFilePlugin::~AudioFilePlugin()
{
    stop();
    _logger->info("Goodbye!");
}

void AudioFilePlugin::thread()
{
    using namespace std::chrono_literals;

    while (_running)
    {
        std::this_thread::sleep_for(1ms);

        auto sample_count = _audioFile.getSampleRate() / 1000.0;
        for (int i = 0; i < sample_count; ++i)
        {
            if (_current_sample >= static_cast<std::size_t>(_audioFile.getNumSamplesPerChannel()))
            {
                _current_sample = 0;
            }

            auto sample = _audioFile.samples[0][_current_sample];
            _ts->push_samples(sample);

            _current_sample++;
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
    else
    {
        _logger->warn("Already running");
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
    else
    {
        _logger->warn("Already stopped");
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
