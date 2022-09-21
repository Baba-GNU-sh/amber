#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ui.hpp"

using namespace amber;

ImGuiMenuView::ImGuiMenuView(Window_GLFW &window,
                             PluginManager &plugin_manager,
                             Graph &graph,
                             database::Database &database,
                             GraphState &graph_state)
    : m_window(window), m_plugin_manager(plugin_manager), m_graph(graph), m_database(database),
      m_graph_state(graph_state)
{
    update_vsync();
    update_call_glfinish();
    update_multisampling();
    update_bg_colour();
    update_follow_latest_data();
}

void ImGuiMenuView::draw()
{
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
            {
                update_vsync();
            }

            if (ImGui::Checkbox("Multisampling", &m_enable_multisampling))
            {
                update_multisampling();
            }

            if (ImGui::Checkbox("Call glFinish", &m_call_glfinish))
            {
                update_call_glfinish();
            }

            ImGui::Separator();

            if (ImGui::ColorEdit3(
                    "Clear colour", &(m_clear_colour.x), ImGuiColorEditFlags_NoInputs))
            {
                update_bg_colour();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Graph"))
        {
            if (ImGui::MenuItem("Go to newst sample", "Space"))
            {
                m_graph.reveal_newest_sample();
            }

            if (ImGui::Checkbox("Sync with latest data", &m_follow_latest_data))
            {
                update_follow_latest_data();
            }

            ImGui::SliderInt("Line width", &m_graph_state.plot_width, 1, 4);
            ImGui::Checkbox("Show line segments", &m_graph_state.show_line_segments);

            ImGui::Separator();

            if (!m_graph.marker_is_visible(Graph::MarkerType::A))
            {
                if (ImGui::MenuItem("Show Marker A", "A"))
                {
                    const auto marker_pos_gs =
                        m_graph.get_view_transform().apply(glm::vec2(0.0, 0.0));
                    m_graph.set_marker_position(Graph::MarkerType::A, marker_pos_gs.x);
                    m_graph.set_marker_visible(Graph::MarkerType::A, true);
                }
            }
            else
            {
                if (ImGui::MenuItem("Hide Marker A", "Ctrl+A"))
                {
                    m_graph.set_marker_visible(Graph::MarkerType::A, false);
                }
            }

            if (!m_graph.marker_is_visible(Graph::MarkerType::B))
            {
                if (ImGui::MenuItem("Show Marker B", "B"))
                {
                    const auto marker_pos_gs =
                        m_graph.get_view_transform().apply(glm::vec2(0.0, 0.0));
                    m_graph.set_marker_position(Graph::MarkerType::B, marker_pos_gs.x);
                    m_graph.set_marker_visible(Graph::MarkerType::B, true);
                }
            }
            else
            {
                if (ImGui::MenuItem("Hide Marker B", "Ctrl+B"))
                {
                    m_graph.set_marker_visible(Graph::MarkerType::B, false);
                }
            }

            if (m_graph.marker_is_visible(Graph::MarkerType::A) ||
                m_graph.marker_is_visible(Graph::MarkerType::B))
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Hide Markers", "C"))
                {
                    m_graph.set_marker_visible(Graph::MarkerType::A, false);
                    m_graph.set_marker_visible(Graph::MarkerType::B, false);
                }
            }

            ImGui::Separator();

            for (auto &plugin : m_graph_state.timeseries)
            {
                // Im ImGui, widgets need unique label names
                // Anything after the "##" is counted towards the uniqueness but is not
                // displayed
                const auto label_name = "##" + plugin.name;
                ImGui::Checkbox(label_name.c_str(), &(plugin.visible));
                ImGui::SameLine();
                ImGui::ColorEdit3(
                    plugin.name.c_str(), &(plugin.colour.x), ImGuiColorEditFlags_NoInputs);
                const auto slider_name = "Y offset##" + plugin.name;
                ImGui::DragFloat(slider_name.c_str(), &(plugin.y_offset), 0.01);
            }

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
            ImGui::SetWindowPos(
                ImVec2(m_window.window_size().x - ImGui::GetWindowWidth() - 10, menubar_size.y),
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
                imgui_print_matrix(m_graph.get_view_transform().matrix());

                ImGui::Text("Viewport Matrix:");
                imgui_print_matrix(m_window.viewport_transform().matrix());

                const auto &view_matrix = m_graph.get_view_transform().matrix();
                ImGui::Text("View Matrix:");
                for (int i = 0; i < 3; i++)
                {
                    ImGui::Text(
                        " %f %f %f", view_matrix[0][i], view_matrix[1][i], view_matrix[2][i]);
                }

                const auto cursor_gs = m_graph.cursor_gs();
                ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);

                if (m_graph.marker_is_visible(Graph::MarkerType::A))
                    ImGui::Text("Marker A: %0.3f", m_graph.marker_position(Graph::MarkerType::A));

                if (m_graph.marker_is_visible(Graph::MarkerType::B))
                    ImGui::Text("Marker B: %0.3f", m_graph.marker_position(Graph::MarkerType::B));

                if (m_graph.marker_is_visible(Graph::MarkerType::A) &&
                    m_graph.marker_is_visible(Graph::MarkerType::B))
                {
                    const auto marker_interval = m_graph.marker_position(Graph::MarkerType::B) -
                                                 m_graph.marker_position(Graph::MarkerType::A);
                    ImGui::Text("Marker Interval: %0.3f, %0.1fHz",
                                marker_interval,
                                1.0 / std::abs(marker_interval));
                }
            }

            if (ImGui::CollapsingHeader("Database", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // Show database stats
                auto [total_bytes, suffix] =
                    human_readable(m_database.memory_usage(), 1024, {"KiB", "MiB", "GiB", "TiB"});
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

void ImGuiMenuView::update_vsync() const
{
    glfwSwapInterval(m_enable_vsync ? 1 : 0);
}

void ImGuiMenuView::update_call_glfinish()
{
    m_window.set_call_glfinish(m_call_glfinish);
}

void ImGuiMenuView::update_multisampling()
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

void ImGuiMenuView::update_bg_colour()
{
    m_window.set_bg_colour(m_clear_colour);
}

void ImGuiMenuView::update_follow_latest_data()
{
    m_graph.set_follow_latest_data(m_follow_latest_data);
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
std::pair<double, const char *> ImGuiMenuView::human_readable(std::size_t size,
                                                              double divisor,
                                                              std::vector<const char *> suffixes)
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
