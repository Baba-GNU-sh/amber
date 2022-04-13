#include "random_data_plugin.hpp"

#include <random>
#include <chrono>
#include <cmath>
#include <imgui.h>
#include <ratio>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdexcept>

RandomDataPlugin::RandomDataPlugin(PluginContext &ctx) : _ctx(ctx)
{
    _logger = spdlog::stdout_color_mt("RandomDataPlugin");
    _logger->info("Initialized");

    _ts = std::make_shared<TimeSeriesSparse>();
    _ctx.get_database().register_timeseries("random/channel", _ts);
}

RandomDataPlugin::~RandomDataPlugin()
{
    if (_running)
    {
        _running = false;
        _thread.join();
    }
    _logger->info("Bye");
}

void RandomDataPlugin::start()
{
    if (!_running)
    {
        _thread = std::thread(&RandomDataPlugin::thread_handler, this);
        _running = true;
        _logger->info("Started");
    }
}

void RandomDataPlugin::stop()
{
    if (_running)
    {
        _running = false;
        _thread.join();
        _logger->info("Stopped");
    }
}

bool RandomDataPlugin::is_running() const
{
    return _running;
}

void RandomDataPlugin::draw_menu()
{
    //
}

void RandomDataPlugin::thread_handler()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    std::mt19937_64 eng{std::random_device{}()};
    std::uniform_int_distribution<> dist{10, 100};

    auto start_time = std::chrono::steady_clock::now();

    while (_running)
    {
        auto sleep_time_millis = dist(eng);
        std::this_thread::sleep_for(microseconds(sleep_time_millis));

        const auto now = steady_clock::now();
        auto delta = std::chrono::duration<double>(now - start_time);

        _ts->push_sample(Sample{delta.count(), static_cast<double>(sleep_time_millis) / 1000});
    }
}
