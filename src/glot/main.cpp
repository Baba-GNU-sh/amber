#include <boost/signals2/connection.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <spdlog/spdlog.h>
#include <database/database.hpp>
#include "plot.hpp"
#include "imgui_database_panel.hpp"
#include "plugin_context.hpp"
#include "plugin_manager.hpp"
#include "window.hpp"
#include <imgui.h>
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include "imgui_window.hpp"
#include "graph.hpp"
#include "audiofile_plugin.hpp"
#include "wavegen_plugin.hpp"

static bool m_call_glfinish = false;
static bool m_enable_vsync = true;
static bool m_enable_multisampling = true;
static glm::vec3 m_clear_colour(0.1, 0.1, 0.1);

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

template <class T> static void imgui_print_matrix(const T &m)
{
    for (int i = 0; i < 3; i++)
    {
        ImGui::Text(" %f %f %f", m[0][i], m[1][i], m[2][i]);
    }
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

void init_timeseries(Database &database, GraphState &state)
{
    const auto createGlmColour = [](int code) {
        float r = static_cast<float>(0xFF & (code >> 16)) / 256.0f;
        float g = static_cast<float>(0xFF & (code >> 8)) / 256.0f;
        float b = static_cast<float>(0xFF & (code >> 0)) / 256.0f;
        return glm::vec3(r, g, b);
    };

    const auto pick_next_colour = [](auto begin, auto end, auto current) {
        current++;
        if (current == end)
        {
            current = begin;
        }
        return current;
    };

    // A nice selection of material colours from here (column 400):
    // https://material.io/resources/color/
    std::vector<glm::vec3> plot_colours;

    plot_colours.push_back(createGlmColour(0xef5350));
    plot_colours.push_back(createGlmColour(0x42a5f5));
    plot_colours.push_back(createGlmColour(0xd4e157));
    plot_colours.push_back(createGlmColour(0xec407a));
    plot_colours.push_back(createGlmColour(0x26c6da));
    plot_colours.push_back(createGlmColour(0xffee58));
    plot_colours.push_back(createGlmColour(0xab47bc));
    plot_colours.push_back(createGlmColour(0x26a69a));
    plot_colours.push_back(createGlmColour(0xffca28));
    plot_colours.push_back(createGlmColour(0x7e57c2));
    plot_colours.push_back(createGlmColour(0x66bb6a));
    plot_colours.push_back(createGlmColour(0xffa726));
    plot_colours.push_back(createGlmColour(0x5c6bc0));
    plot_colours.push_back(createGlmColour(0x9ccc65));
    plot_colours.push_back(createGlmColour(0xff7043));

    const auto &data = database.data();
    state.timeseries.resize(data.size());
    auto colour = plot_colours.begin();
    std::transform(data.begin(),
                   data.end(),
                   state.timeseries.begin(),
                   [&colour, &plot_colours, &pick_next_colour](const auto &ts) {
                       GraphState::TimeSeriesState cont;
                       cont.name = ts.first;
                       cont.ts = ts.second;
                       cont.colour = *colour;
                       colour = pick_next_colour(plot_colours.begin(), plot_colours.end(), colour);
                       cont.visible = true;
                       cont.y_offset = 0.0f;
                       return cont;
                   });
}

class KeyController : public View
{
  public:
    KeyController(Window &window, Graph &graph, GraphState &graph_state)
        : m_window(window), m_graph(graph), m_graph_state(graph_state)
    {
        //
    }

    void on_key(Window &, int key, int, int action, int mods) override
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
            {
                m_window.set_fullscreen(!m_window.is_fullscreen());
            }
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                m_window.request_close();
            }
            if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
            {
                m_graph_state.goto_newest_sample();
            }
            if (key == GLFW_KEY_A && action == GLFW_PRESS)
            {
                if (mods == GLFW_MOD_CONTROL)
                {
                    m_graph_state.markers.first.visible = false;
                }
                else
                {
                    const auto cursor_gs = m_graph.cursor_gs();
                    m_graph_state.markers.first.position = cursor_gs.x;
                    m_graph_state.markers.first.visible = true;
                }
            }
            else if (key == GLFW_KEY_B && action == GLFW_PRESS)
            {
                if (mods == GLFW_MOD_CONTROL)
                {
                    m_graph_state.markers.second.visible = false;
                }
                else
                {
                    const auto cursor_gs = m_graph.cursor_gs();
                    m_graph_state.markers.second.position = cursor_gs.x;
                    m_graph_state.markers.second.visible = true;
                }
            }
            else if (key == GLFW_KEY_C && action == GLFW_PRESS)
            {
                m_graph_state.markers.first.visible = false;
                m_graph_state.markers.second.visible = false;
            }
        }
    }

  private:
    Window &m_window;
    Graph &m_graph;
    GraphState &m_graph_state;
};

class ImGuiMenuView : public View
{
  public:
    ImGuiMenuView(Window &window,
                  PluginManager &plugin_manager,
                  Graph &graph,
                  Database &database,
                  GraphState &graph_state)
        : m_window(window), m_plugin_manager(plugin_manager), m_graph(graph), m_database(database),
          m_graph_state(graph_state)
    {
        //
    }

    void draw(const Window &) const override
    {
        ImGuiDatabasePanel panel(m_graph_state);

        ImVec2 menubar_size;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close", "Esc"))
                {
                    m_window.request_close();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Toggle Fullscreen", "F11"))
                    m_window.set_fullscreen(!m_window.is_fullscreen());

                ImGui::Separator();

                if (ImGui::Checkbox("Enable VSync", &m_enable_vsync))
                    update_vsync();

                if (ImGui::Checkbox("Multisampling", &m_enable_multisampling))
                    update_multisampling();

                ImGui::Checkbox("Call glFinish", &m_call_glfinish);

                ImGui::Separator();

                if (ImGui::ColorEdit3(
                        "Clear colour", &(m_clear_colour.x), ImGuiColorEditFlags_NoInputs))
                    m_window.set_bg_colour(m_clear_colour);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Graph"))
            {
                if (ImGui::MenuItem("Go to newst sample", "Space"))
                {
                    m_graph_state.goto_newest_sample();
                }

                ImGui::Checkbox("Sync with latest data", &m_graph_state.sync_latest_data);
                ImGui::SliderInt("Line width", &m_graph_state.plot_width, 1, 4);
                ImGui::Checkbox("Show line segments", &m_graph_state.show_line_segments);

                ImGui::Separator();

                if (!m_graph_state.markers.first.visible)
                {
                    if (ImGui::MenuItem("Show Marker A", "A"))
                    {
                        const auto marker_pos_gs = m_graph_state.view.apply(glm::vec2(0.0, 0.0));
                        m_graph_state.markers.first.position = marker_pos_gs.x;
                        m_graph_state.markers.first.visible = true;
                    }
                }
                else
                {
                    if (ImGui::MenuItem("Hide Marker A", "Ctrl+A"))
                    {
                        m_graph_state.markers.first.visible = false;
                    }
                }

                if (!m_graph_state.markers.second.visible)
                {
                    if (ImGui::MenuItem("Show Marker B", "B"))
                    {
                        const auto marker_pos_gs = m_graph_state.view.apply(glm::vec2(0.0, 0.0));
                        m_graph_state.markers.second.position = marker_pos_gs.x;
                        m_graph_state.markers.second.visible = true;
                    }
                }
                else
                {
                    if (ImGui::MenuItem("Hide Marker B", "Ctrl+B"))
                    {
                        m_graph_state.markers.second.visible = false;
                    }
                }

                if (m_graph_state.markers.first.visible || m_graph_state.markers.second.visible)
                {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Hide Markers", "C"))
                    {
                        m_graph_state.markers.first.visible = false;
                        m_graph_state.markers.second.visible = false;
                    }
                }

                ImGui::Separator();

                panel.draw_imgui_panel();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Plugins"))
            {
                m_plugin_manager.draw_menu();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                ImGui::BulletText("Left mouse + drag to pan");
                ImGui::BulletText("Scroll to zoom");
                ImGui::BulletText("Scroll on gutters to zoom individual axes");
                ImGui::BulletText("Press A or B to place markers");
                ImGui::EndMenu();
            }

            menubar_size = ImGui::GetWindowSize();
            ImGui::EndMainMenuBar();
        }

        {
            ImGui::Begin("Info",
                         0,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove);
            {
                ImGui::SetWindowPos(ImVec2(800 - ImGui::GetWindowWidth() - 10, menubar_size.y),
                                    true);

                if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text("%.1f ms/frame (%.1f FPS)",
                                1000.0f / ImGui::GetIO().Framerate,
                                ImGui::GetIO().Framerate);
                }

                if (ImGui::CollapsingHeader("Graph", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Text("View Matrix:");
                    imgui_print_matrix(m_graph_state.view.matrix());

                    ImGui::Text("Viewport Matrix:");
                    imgui_print_matrix(m_window.viewport_transform().matrix());

                    const auto &view_matrix = m_graph_state.view.matrix();
                    ImGui::Text("View Matrix:");
                    for (int i = 0; i < 3; i++)
                    {
                        ImGui::Text(
                            " %f %f %f", view_matrix[0][i], view_matrix[1][i], view_matrix[2][i]);
                    }

                    const auto cursor_gs = m_graph.cursor_gs();
                    ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);

                    const auto &marker_a = m_graph_state.markers.first;
                    const auto &marker_b = m_graph_state.markers.second;

                    if (marker_a.visible)
                        ImGui::Text("Marker A: %0.3f", marker_a.position);

                    if (marker_b.visible)
                        ImGui::Text("Marker B: %0.3f", marker_b.position);

                    if (marker_a.visible && marker_b.visible)
                    {
                        const auto marker_interval = marker_b.position - marker_a.position;
                        ImGui::Text("Marker Interval: %0.3f, %0.1fHz",
                                    marker_interval,
                                    1.0 / std::abs(marker_interval));
                    }
                }

                if (ImGui::CollapsingHeader("Database", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    // Show database stats
                    auto [total_bytes, suffix] = human_readable(
                        m_database.memory_usage(), 1024, {"KiB", "MiB", "GiB", "TiB"});
                    ImGui::Text("Memory Used: %.1f%s", total_bytes, suffix);

                    auto [total_samples, samples_suffix] =
                        human_readable(m_database.num_samples(), 1000, {"K", "M", "B"});
                    ImGui::Text("Total Samples: %.1f%s", total_samples, samples_suffix);

                    ImGui::Text("Bytes/sample: %.1f",
                                static_cast<double>(m_database.memory_usage()) /
                                    static_cast<double>(m_database.num_samples()));
                }

                m_plugin_manager.draw_dialogs();
            }

            ImGui::End();
        }
    }

  private:
    Window &m_window;
    PluginManager &m_plugin_manager;
    Graph &m_graph;
    Database &m_database;
    GraphState &m_graph_state;
};

int main()
{
    spdlog::info("Initializing...");

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

        // We need to do this after creating our GL context which is done when the first GLFW
        // window is created
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        GraphState state;
        init_timeseries(db, state);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        update_multisampling();
        update_vsync();
        window.set_bg_colour(m_clear_colour);

        Graph graph(state, window);
        window.add_view(&graph);

        KeyController key_controller(window, graph, state);
        window.add_view(&key_controller);

        ImGuiMenuView gui(window, plugin_manager, graph, db, state);
        window.add_imgui_view(&gui);

        spdlog::info("Initialization OK, starting main loop");

        window.init();

        // Main loop
        while (!window.should_close())
        {
            glfwPollEvents();
            window.render();

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
