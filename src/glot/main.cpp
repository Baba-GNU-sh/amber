#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "graph.hpp"
#include "database.hpp"
#include "plugin_context.hpp"
#include "plugin_manager.hpp"
#include "window.hpp"
#include <imgui.h>
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include "imgui_window.hpp"
#include "app_context.hpp"
#include "audiofile_plugin.hpp"
#include "wavegen_plugin.hpp"

void error_callback(int error, const char *msg)
{
    spdlog::error("[{}] {}", error, msg);
}

int main()
{
    spdlog::info("Initializing...");

    glfwSetErrorCallback(error_callback);

    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // #ifdef __APPLE__
    //     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // #endif

    try
    {
        Database db;
        PluginContext plugin_context(db);
        PluginManager plugin_manager;
        plugin_manager.add_plugin("audiofile", std::make_shared<AudioFilePlugin>(plugin_context, "audio/Lurking_Threat_3.wav"));
        plugin_manager.add_plugin("wavegen", std::make_shared<WaveGenPlugin>(plugin_context));
        plugin_manager.start_all();

        ImGuiContextWindow window(800, 600, "GLot");

        // We need to do this after creating our GL context which is done when the first GLFW window
        // is created
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Our graph requires depth testing to be enabled in order to render correcly
        glEnable(GL_DEPTH_TEST);

        // Blending should be enabled for text to be rendered correclty
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        Graph graph(window);
        Plot plot(window);
        AppContext context(db, graph, plot, window, plugin_manager);

        spdlog::info("Initialization OK, starting main loop");

        // Main loop go brr
        while (!window.should_close())
        {
            glfwPollEvents();

            window.use();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            context.draw();
            window.finish();
        }
    }
    catch (const std::exception &e)
    {
        spdlog::error("Error: {}", e.what());
    }

    glfwTerminate();

    spdlog::info("Shutting down normally");

    return 0;
}
