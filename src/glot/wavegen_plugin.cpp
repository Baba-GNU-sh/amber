#include "wavegen_plugin.hpp"

#include <chrono>
#include <cmath>
#include <imgui.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

WaveGenPlugin::WaveGenPlugin(PluginContext &ctx) : m_ctx(ctx)
{
    m_logger = spdlog::stdout_color_mt("WaveGenPlugin");
    m_logger->info("Initialized");

    m_ts = std::make_shared<TimeSeriesDense>(0.0, 1.0 / m_sample_rate);
    m_ctx.get_database().register_timeseries("wavegen/channelA", m_ts);

    m_settings.amplitude = 1.0;
    m_settings.frequency = 1.0;
    m_settings.type = Sine;
}

WaveGenPlugin::~WaveGenPlugin()
{
    if (m_running)
    {
        m_running = false;
        m_thread.join();
    }
    m_logger->info("Bye");
}

void WaveGenPlugin::start()
{
    if (!m_running)
    {
        m_running = true;
        m_thread = std::thread(&WaveGenPlugin::thread_handler, this);
        m_logger->info("Started");
    }
}

void WaveGenPlugin::stop()
{
    if (m_running)
    {
        m_running = false;
        m_thread.join();
        m_logger->info("Stopped");
    }
}

bool WaveGenPlugin::is_running() const
{
    return m_running;
}

void WaveGenPlugin::draw_menu()
{
    const char *items[] = {"Sine", "Square", "Triangle", "SawTooth"};

    ImGui::Begin("Wave Gen");
    ImGui::Text("Running: %s", m_running ? "yes" : "no");
    ImGui::Text("Sample Rate: %u", m_sample_rate);

    std::lock_guard<std::mutex> _(m_mutex);
    ImGui::Combo(
        "Signal Type", reinterpret_cast<int *>(&m_settings.type), items, IM_ARRAYSIZE(items));
    ImGui::SliderFloat(
        "Frequency", &m_settings.frequency, 0.1, 1000, "%.1f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat(
        "Amplitude", &m_settings.amplitude, 0.1, 10.0, "%.3f", ImGuiSliderFlags_Logarithmic);

    ImGui::End();
}

void WaveGenPlugin::thread_handler()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    const auto sample_period = std::chrono::duration<double>(1s) / m_sample_rate;
    auto prevtime = std::chrono::steady_clock::now();
    double x = 0.0;

    while (m_running)
    {
        std::this_thread::sleep_for(10ms);

        const auto now = steady_clock::now();
        auto delta = std::chrono::duration<double>(now - prevtime);
        prevtime = now;

        WaveSettings settings = [this]() {
            std::lock_guard<std::mutex> _(m_mutex);
            return m_settings;
        }();

        while (delta > seconds(0))
        {
            x += settings.frequency / m_sample_rate;
            const auto value = settings.amplitude * sample_value(settings.type, x);
            m_ts->push_sample(value);
            delta -= sample_period;
        }
    }
}

double WaveGenPlugin::sample_value(WaveType type, double time) const
{
    const double M_2PI = 2 * M_PI;
    const double radians = M_2PI * time;
    switch (type)
    {
    case Sine:
        return std::sin(radians);
    case Square:
        return std::sin(radians) > 0.0 ? 1.0 : -1.0;
    case Triangle:
        return 2 * std::asin(std::sin(radians)) / M_PI;
    case SawTooth:
        return 2 * std::atan(std::tan(radians / 2)) / M_PI;
    default:
        throw std::invalid_argument("Bad wave type");
    }
}
