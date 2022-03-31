#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "audiofile_plugin.hpp"
#include "window.hpp"

#include "database.hpp"
#include "plugin_context.hpp"
#include "plugin_manager.hpp"

void error_callback(int error, const char *msg)
{
    spdlog::error("[{}] {}", error, msg);
}

int main()
{
    glfwSetErrorCallback(error_callback);

    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    spdlog::info("Initializing...");

    try
    {
        Database db;
        PluginContext plugin_context(db);
        PluginManager plugin_manager(plugin_context);

        spdlog::info("There are {} timeseries in the database", db.data().size());

        Window win(db);
        // win.set_data(audio.data(), audio.size(), audio.sample_rate());
        spdlog::info("Initialization OK: Spinning forever");
        win.spin();
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error: {}", e.what());
    }

    glfwTerminate();

    spdlog::info("Shutting down normally");

    return 0;
}
