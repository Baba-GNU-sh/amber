#include "audiofile_plugin.hpp"

#include <imgui.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdexcept>

using namespace amber;

AudioFilePlugin::AudioFilePlugin(PluginContext &pluggy, std::string_view filename)
    : m_running(false), m_current_sample(0), m_filename(filename)
{
    if (!m_audioFile.load(std::string(filename)))
    {
        throw std::runtime_error("Unable to load audio file");
    }

    auto ts = std::make_shared<database::TimeSeriesDense>(0.0, 1.0 / m_audioFile.getSampleRate());
    pluggy.get_database().register_timeseries(std::string(filename), ts);
    m_ts = ts;

    m_logger = spdlog::stdout_color_mt("AudioFilePlugin");
    m_logger->info("Loaded audio file {}", filename);
    m_logger->info("Sample rate: {}", m_audioFile.getSampleRate());
    m_logger->info("Total samples: {}", m_audioFile.getNumSamplesPerChannel());
    m_logger->info("Initialized");
}

AudioFilePlugin::~AudioFilePlugin()
{
    if (m_running)
    {
        m_running = false;
        m_thread.join();
    }
    m_logger->info("Goodbye!");
}

void AudioFilePlugin::background_worker()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    const auto sample_period = std::chrono::duration<double>(1s) / m_audioFile.getSampleRate();
    auto prevtime = std::chrono::steady_clock::now();

    while (m_running)
    {
        std::this_thread::sleep_for(10ms);

        const auto now = steady_clock::now();
        auto delta = std::chrono::duration<double>(now - prevtime);
        prevtime = now;

        while (delta > seconds(0))
        {
            if (m_current_sample >= static_cast<std::size_t>(m_audioFile.getNumSamplesPerChannel()))
            {
                m_current_sample = 0;
            }

            auto sample = m_audioFile.samples[0][m_current_sample++];
            m_ts->push_sample(sample);

            delta -= sample_period;
        }
    }
}

void AudioFilePlugin::start()
{
    if (!m_running)
    {
        m_running = true;
        m_thread = std::thread(&AudioFilePlugin::background_worker, this);
        m_logger->info("Started");
    }
}

void AudioFilePlugin::stop()
{
    if (m_running)
    {
        m_running = false;
        m_thread.join();
        m_logger->info("Stopped");
    }
}

bool AudioFilePlugin::is_running() const
{
    return m_running;
}

void AudioFilePlugin::draw_menu()
{
    ImGui::Begin("AudioFile");
    ImGui::Text("File: %s", m_filename.c_str());
    ImGui::Text("Sample Rate: %u", m_audioFile.getSampleRate());
    ImGui::End();
}
