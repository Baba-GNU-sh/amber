#include "graph_controller.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "database.hpp"
#include "plugin_manager.hpp"
#include "window.hpp"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

GraphController::GraphController(Database &database,
                                 GraphRendererOpenGL &graph,
                                 Window &window,
                                 PluginManager &plugin_manager)
    : m_database(database), m_graph(graph), m_window(window), m_plugin_manager(plugin_manager)
{
    std::vector<glm::vec3> plot_colours = {
        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};

    const auto &data = m_database.data();
    m_ts.resize(data.size());
    auto colour = plot_colours.begin();
    std::transform(
        data.begin(), data.end(), m_ts.begin(), [&colour, &plot_colours](const auto &ts) {
            TimeSeriesContainer cont;
            cont.name = ts.first;
            cont.ts = ts.second;
            cont.colour = *colour++;
            cont.visible = true;
            cont.y_offset = 0.0f;
            if (colour == plot_colours.end())
            {
                colour = plot_colours.begin();
            }
            return cont;
        });

    update_view_matrix(glm::dmat3(1.0));

    m_bgcolor = glm::vec3(0.1, 0.1, 0.1);
    update_multisampling();
    update_vsync();
    update_bgcolour();

    m_window.key.connect([this](int key, int, int action, int) {
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            m_view_matrix[2][0] = 0;
            auto latest = m_database.get_latest_sample_time();
            m_view_matrix = glm::translate(m_view_matrix, glm::dvec2(-latest, 0));
        }
        else if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
        {
            m_window.set_fullscreen(!m_window.is_fullscreen());
        }
        else if (key == GLFW_KEY_A && action == GLFW_PRESS)
        {
            auto cursor_gs = screen2graph(m_window.cursor());
            m_markers.first = cursor_gs.x;
        }
        else if (key == GLFW_KEY_B && action == GLFW_PRESS)
        {
            auto cursor_gs = screen2graph(m_window.cursor());
            m_markers.second = cursor_gs.x;
        }
        else if (key == GLFW_KEY_C && action == GLFW_PRESS)
        {
            m_markers.first.reset();
            m_markers.second.reset();
        }
    });

    // Register for mouse events from the window
    m_window.scroll.connect([this](double /*xoffset*/, double yoffset) {
        const double zoom_delta = 1.0f + (yoffset / 10.0f);
        const auto cursor = m_window.cursor();
        const auto size = m_window.size();

        if (hit_test(
                cursor, glm::ivec2(0, 0), glm::ivec2(GUTTER_SIZE_PX, size.y - GUTTER_SIZE_PX)))
        {
            // Cursor is in the vertical gutter, only zoom the y axis
            on_zoom(1.0, zoom_delta);
        }
        else if (hit_test(cursor,
                          glm::ivec2(GUTTER_SIZE_PX, size.y - GUTTER_SIZE_PX),
                          glm::ivec2(size.x, size.y)))
        {
            // Cursor is in the horizontal gutter, only zoom the x axis
            on_zoom(zoom_delta, 1.0);
        }
        else if (hit_test(cursor,
                          glm::ivec2(GUTTER_SIZE_PX, 0),
                          glm::ivec2(size.x, size.y - GUTTER_SIZE_PX)))
        {
            // Cursor is in the main part of the graph, zoom both axes
            on_zoom(zoom_delta, zoom_delta);
        }
    });

    m_window.cursor_pos.connect([this](double xpos, double ypos) {
        glm::dvec2 cursor(xpos, ypos);

        // Work out how much the cursor moved since the last time
        const auto cursor_delta = cursor - m_cursor_old;

        // Work out the delta in graph space
        const auto txform = m_view_matrix_inv * glm::dmat3(m_window.vp_matrix_inv());
        const auto a = txform * glm::dvec3(0.0f);
        const auto b = txform * glm::dvec3(cursor_delta, 0.0f);
        const auto delta = b - a;

        // Convert the delta back to a 2D vector
        glm::dvec2 cursor_gs_delta(delta.x, delta.y);

        if (m_is_dragging)
        {
            update_view_matrix(glm::translate(m_view_matrix, cursor_gs_delta));
        }

        // for (auto &marker : m_markers)
        // {
        //     if (marker.second.is_dragging)
        //     {
        //         *marker.second.position = *marker.second.position + cursor_gs_delta.x;
        //     }
        // }

        // Cache the position of the cursor for next time
        m_cursor_old = cursor;
    });

    m_window.mouse_button.connect([this](int button, int action, int /*mods*/) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            // for (auto &marker : m_markers)
            // {
            //     const auto marker_pos = *marker.second.position;
            //     glm::vec3 marker_pos_vs =
            //         m_window.vp_matrix() * (*m_view_matrix * glm::dvec3(marker_pos, 0.0, 1.0));
            //     auto cursor = m_window.cursor();
            //     if (std::abs(cursor.x - marker_pos_vs.x) < 5)
            //     {
            //         spdlog::info("Clicked on marker {}", marker.first);
            //         marker.second.is_dragging = true;
            //         return;
            //     }
            // }
            m_is_dragging = true;
            return;
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            m_is_dragging = false;
            // for (auto &marker : m_markers)
            // {
            //     marker.second.is_dragging = false;
            // }
        }
    });
}

void GraphController::draw()
{
    m_graph.set_gutter_size(60);
    m_graph.set_tick_len(5);
    m_graph.set_view_matrix(m_view_matrix);
    m_graph.set_size(m_window.size());

    m_graph.draw_graph();

    for (auto &time_series : m_ts)
    {
        if (time_series.visible)
        {
            m_graph.draw_plot(*(time_series.ts),
                              m_plot_width,
                              time_series.colour,
                              time_series.y_offset,
                              m_show_line_segments);
        }
    }

    if (m_markers.first)
    {
        m_graph.draw_marker(
            "A", m_markers.first.value(), MarkerStyle::Left, glm::vec3(0.0, 1.0, 1.0));
    }

    if (m_markers.second)
    {
        m_graph.draw_marker(
            "B", m_markers.second.value(), MarkerStyle::Right, glm::vec3(1.0, 1.0, 0.0));
    }

    draw_gui();
}

void GraphController::spin()
{
    while (!m_window.should_close())
    {
        glfwPollEvents();
        m_window.use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw();
        m_window.finish();

        if (m_call_glfinish)
        {
            glFinish();
        }
    }
}

void GraphController::draw_gui()
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
                m_window.request_close();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Toggle Fullscreen", "F11"))
            {
                m_window.set_fullscreen(!m_window.is_fullscreen());
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Plugins"))
        {
            m_plugin_manager.draw_menu();
            ImGui::EndMenu();
        }

        menubar_size = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }

    ImGui::Begin("Info",
                 0,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(m_window.size().x - ImGui::GetWindowWidth() - 10, menubar_size.y),
                        true);

    if (ImGui::CollapsingHeader("Help", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::BulletText("Left mouse + drag to pan");
        ImGui::BulletText("Scroll to zoom");
        ImGui::BulletText("Scroll on gutters to zoom individual axes");
    }

    if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("%.1f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);

        ImGui::Text("View Matrix:");
        for (int i = 0; i < 3; i++)
        {
            ImGui::Text("%f %f %f", m_view_matrix[0][i], m_view_matrix[1][i], m_view_matrix[2][i]);
        }

        glm::dmat3 vp_matrix_inv = m_window.vp_matrix_inv();
        const auto cursor_gs =
            glm::inverse(m_view_matrix) * vp_matrix_inv * glm::dvec3(m_window.cursor(), 1.0);
        ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);

        if (m_markers.first && m_markers.second)
        {
            const auto marker_interval = *m_markers.second - *m_markers.first;
            ImGui::Text("Markers: %.3fs, %.1fHz", marker_interval, 1.0 / std::abs(marker_interval));
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

        if (ImGui::ColorEdit3("Clear colour", &(m_bgcolor.x)))
        {
            update_bgcolour();
        }

        ImGui::SliderInt("Line width", &m_plot_width, 1, 64, "%d", ImGuiSliderFlags_Logarithmic);

        ImGui::Checkbox("Show line segments", &m_show_line_segments);

        ImGui::Checkbox("Call glFinish", &m_call_glfinish);
    }

    if (ImGui::CollapsingHeader("Plots", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto &plugin : m_ts)
        {
            // Im ImGui, widgets need unique label names
            // Anything after the "##" is counted towards the uniqueness but is not displayed
            const auto label_name = "##" + plugin.name;
            ImGui::Checkbox(label_name.c_str(), &(plugin.visible));
            ImGui::SameLine();
            ImGui::ColorEdit3(
                plugin.name.c_str(), &(plugin.colour.x), ImGuiColorEditFlags_NoInputs);
            const auto slider_name = "Y offset##" + plugin.name;
            ImGui::DragFloat(slider_name.c_str(), &(plugin.y_offset), 0.01);
        }
    }

    m_plugin_manager.draw_dialogs();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphController::update_multisampling() const
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

void GraphController::update_vsync() const
{
    glfwSwapInterval(m_enable_vsync ? 1 : 0);
}

void GraphController::update_bgcolour() const
{
    m_window.set_bg_colour(m_bgcolor);
}

glm::dvec2 GraphController::screen2graph(const glm::ivec2 &value) const
{
    const glm::dvec3 value3(value, 1.0f);
    glm::dvec3 value_cs = m_window.vp_matrix_inv() * value3;
    glm::dvec3 value_gs = glm::inverse(m_view_matrix) * value_cs;
    return value_gs;
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
std::pair<double, const char *> GraphController::human_readable(std::size_t size,
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

void GraphController::on_zoom(double x, double y)
{
    glm::dvec2 zoom_delta_vec(x, y);

    // Work out where the pointer is in graph space
    auto cursor_in_gs_old = screen2graph(m_window.cursor());
    update_view_matrix(glm::scale(m_view_matrix, zoom_delta_vec));

    // Limit zoom
    m_view_matrix[0][0] = std::min(m_view_matrix[0][0], ZOOM_MIN_X);
    m_view_matrix[1][1] = std::min(m_view_matrix[1][1], ZOOM_MIN_Y);

    auto cursor_in_gs_new = screen2graph(m_window.cursor());
    auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;
    update_view_matrix(glm::translate(m_view_matrix, cursor_delta));
}

bool GraphController::hit_test(glm::ivec2 value, glm::ivec2 tl, glm::ivec2 br)
{
    if (value.x < tl.x)
        return false;
    if (value.x >= br.x)
        return false;
    if (value.y < tl.y)
        return false;
    if (value.y >= br.y)
        return false;
    return true;
}

void GraphController::update_view_matrix(const glm::dmat3 &new_view_matrix)
{
    m_view_matrix = new_view_matrix;
    m_view_matrix_inv = glm::inverse(new_view_matrix);
}
