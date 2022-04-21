#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "graph_renderer_opengl.hpp"
#include "database.hpp"
#include "plugin_context.hpp"
#include "plugin_manager.hpp"
#include "window.hpp"
#include <imgui.h>
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include "imgui_window.hpp"
#include "graph_controller.hpp"
#include "audiofile_plugin.hpp"
#include "wavegen_plugin.hpp"

static bool m_call_glfinish = false;
static bool m_enable_vsync = true;
static bool m_enable_multisampling = true;
static glm::vec3 m_clear_colour(0.1, 0.1, 0.1);

static void error_callback(int error, const char *msg)
{
    spdlog::error("[{}] {}", error, msg);
}

static void update_multisampling()
{
    if (m_enable_multisampling)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }
}

static void update_vsync()
{
    glfwSwapInterval(m_enable_vsync ? 1 : 0);
}

/**
 * @brief Turns an unsigned "size" value into a human readable value with a suffix. Useful for
 * displaying things like number of bytes.
 *
 * @param size The size to process.
 * @param divisor The interval between each suffix. Should be something like 1000 or 1024.
 * @param suffixes A list of suffixes such as "K", "M", "B".
 * @return std::pair<double, const char *> The divided down value, as well as the string suffix.
 */
std::pair<double, const char *> human_readable(std::size_t size,
                                               double divisor = 1000,
                                               std::vector<const char *> suffixes = {
                                                   "K", "M", "B", "T"})
{
    double size_hr = size;
    const char *suffix = "";
    for (auto s : suffixes)
    {
        if (size_hr < divisor)
            break;

        size_hr = size_hr / divisor;
        suffix = s;
    }
    return std::make_pair(size_hr, suffix);
}

static void draw_gui(Window &window,
                     PluginManager &plugin_manager,
                     GraphController &graph_controller,
                     Database &database)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImVec2 menubar_size;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close", "Esc"))
            {
                window.request_close();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Toggle Fullscreen", "F11"))
            {
                window.set_fullscreen(!window.is_fullscreen());
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Graph"))
        {
            graph_controller.draw_menu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Plugins"))
        {
            plugin_manager.draw_menu();
            ImGui::EndMenu();
        }

        menubar_size = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Info",
                 0,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(window.size().x - ImGui::GetWindowWidth() - 10, menubar_size.y),
                        true);

    if (ImGui::CollapsingHeader("Help", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::BulletText("Left mouse + drag to pan");
        ImGui::BulletText("Scroll to zoom");
        ImGui::BulletText("Scroll on gutters to zoom individual axes");
    }

    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("%.1f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
    }

    if (ImGui::CollapsingHeader("Graph", ImGuiTreeNodeFlags_DefaultOpen))
    {
        graph_controller.draw_gui();
    }

    if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Checkbox("Enable VSync", &m_enable_vsync))
        {
            update_vsync();
        }

        if (ImGui::Checkbox("Multisampling", &m_enable_multisampling))
        {
            update_multisampling();
        }

        if (ImGui::ColorEdit3("Clear colour", &(m_clear_colour.x)))
        {
            window.set_bg_colour(m_clear_colour);
        }

        ImGui::Checkbox("Call glFinish", &m_call_glfinish);
    }

    if (ImGui::CollapsingHeader("Database", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Show database stats
        auto [total_bytes, suffix] =
            human_readable(database.memory_usage(), 1024, {"KiB", "MiB", "GiB", "TiB"});
        ImGui::Text("Memory Used: %.1f%s", total_bytes, suffix);

        auto [total_samples, samples_suffix] =
            human_readable(database.num_samples(), 1000, {"K", "M", "B"});
        ImGui::Text("Total Samples: %.1f%s", total_samples, samples_suffix);

        ImGui::Text("Bytes/sample: %.1f",
                    static_cast<double>(database.memory_usage()) /
                        static_cast<double>(database.num_samples()));
    }

    plugin_manager.draw_dialogs();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main()
{
    spdlog::info("Initializing...");

    glfwSetErrorCallback(error_callback);

    glfwInit();

    // #ifdef __APPLE__
    //     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // #endif

    try
    {
        // Create the timeseries database - this is where all the data goes!
        Database db;

        // Create and launch a selection of example plugins
        PluginContext plugin_context(db);
        PluginManager plugin_manager;
        plugin_manager.add_plugin(
            "audiofile",
            std::make_shared<AudioFilePlugin>(plugin_context, "audio/Lurking_Threat_3.wav"));
        plugin_manager.add_plugin("wavegen", std::make_shared<WaveGenPlugin>(plugin_context));
        plugin_manager.start_all();

        // Create a new window using GLFW, OpenGL and initializing ImGui
        ImGuiContextWindow window(1024, 768, "GLot");

        // Listen to the keyboard events for hotkeys
        window.key.connect([&window](int key, int, int action, int) {
            if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
            {
                window.set_fullscreen(!window.is_fullscreen());
            }
        });

        // We need to do this after creating our GL context which is done when the first GLFW window
        // is created
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Our graph renderer requires depth testing, and blending to be enabled in order to render
        // correcly
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        update_multisampling();
        update_vsync();
        window.set_bg_colour(m_clear_colour);

        GraphRendererOpenGL graph_renderer(window);
        GraphController graph_controller(db, graph_renderer, window);

        spdlog::info("Initialization OK, starting main loop");

        // Main loop go brr
        while (!window.should_close())
        {
            glfwPollEvents();

            window.use();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            graph_controller.draw();

            draw_gui(window, plugin_manager, graph_controller, db);

            window.finish();

            if (m_call_glfinish)
            {
                glFinish();
            }
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
