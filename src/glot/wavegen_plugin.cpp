#include "wavegen_plugin.hpp"

#include <chrono>
#include <imgui.h>
#include <spdlog/sinks/stdout_color_sinks.h>

WaveGenPlugin::WaveGenPlugin(PluginContext &ctx) : _ctx(ctx)
{
    _logger = spdlog::stdout_color_mt("WaveGenPlugin");
    _logger->info("Initialized");

    _ts = std::make_shared<TimeSeriesDense>(0.0, 1.0 / _sample_rate);
    _ctx.get_database().register_timeseries("ChannelA", _ts);
}

WaveGenPlugin::~WaveGenPlugin()
{
    stop();
    _logger->info("Bye");
}

void WaveGenPlugin::start()
{
    if (!_running)
    {
        _thread = std::thread(&WaveGenPlugin::thread_handler, this);
        _running = true;
        _logger->info("Started");
    }
}

void WaveGenPlugin::stop()
{
    if (_running)
    {
        _running = false;
        _thread.join();
        _logger->info("Stopped");
    }
}

bool WaveGenPlugin::is_running() const
{
    return _running;
}

void WaveGenPlugin::draw_menu()
{
    ImGui::Begin("Wave Gen");
    ImGui::Text("Sample Rate: %u", _sample_rate);
    ImGui::SliderFloat("Frequency", &_frequency, 0.1, 10);
    const char *items[] = {"Sine", "Square", "Triangle", "SawTooth"};
    ImGui::Combo("Signal Type", &_wave_type, items, IM_ARRAYSIZE(items));
    ImGui::Text("Running: %s", _running ? "yes" : "no");
}

void WaveGenPlugin::thread_handler()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    auto prevtime = std::chrono::steady_clock::now();

    while (_running)
    {
        std::this_thread::sleep_for(10ms);

        const auto timenow = steady_clock::now();
        const auto duration = duration_cast<milliseconds>(timenow - prevtime);
        prevtime = timenow;

        for (int i = 0; i < duration.count(); i++)
        {
            const double time = static_cast<double>(_ticks++) * _frequency / _sample_rate;
            switch (_wave_type)
            {
            case Sine:
                _ts->push_sample(std::sin(time));
                break;
            case Square:
                _ts->push_sample(std::sin(time) > 0.0 ? 1.0 : -1.0);
                break;
            case Triangle:
                _ts->push_sample(2 * std::asin(std::sin(time)) / M_PI);
                break;
            case SawTooth:
                _ts->push_sample(2 * std::atan(std::tan(M_PI_2 - time)) / M_PI);
                break;
            }
        }
    }
}
