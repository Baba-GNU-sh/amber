#include "app_context.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "graph.hpp"
#include "database.hpp"
#include "plugin_manager.hpp"
#include "window.hpp"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

AppContext::AppContext(
    Database &database, Graph &graph, Plot &plot, Window &window, PluginManager &plugin_manager)
    : m_database(database), m_graph(graph), m_plot(plot), m_window(window),
      m_plugin_manager(plugin_manager), m_view_matrix(1.0)
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

    m_bgcolor = glm::vec3(0.1, 0.1, 0.1);
    update_multisampling();
    update_vsync();
    update_bgcolour();

    m_graph.on_drag.connect([this](double x, double y) {
        glm::dmat3 dvp_matrix_inv = m_window.vp_matrix_inv();
        const auto txform = glm::inverse(m_view_matrix) * dvp_matrix_inv;
        const auto a = txform * glm::dvec3(0.0f);
        const auto b = txform * glm::dvec3(x, y, 0.0f);
        const auto delta = b - a;
        glm::dvec2 cursor_gs_delta(delta.x, delta.y);

        m_view_matrix = glm::translate(m_view_matrix, cursor_gs_delta);
    });

    m_graph.on_zoom.connect([this](double x, double y) {
        glm::dvec2 zoom_delta_vec(x, y);

        // Work out where the pointer is in graph space
        auto cursor_in_gs_old = screen2graph(m_window.cursor());
        m_view_matrix = glm::scale(m_view_matrix, zoom_delta_vec);

        // Limit zoom
        m_view_matrix[0][0] = std::min(m_view_matrix[0][0], ZOOM_MIN_X);
        m_view_matrix[1][1] = std::min(m_view_matrix[1][1], ZOOM_MIN_Y);

        auto cursor_in_gs_new = screen2graph(m_window.cursor());
        auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;
        m_view_matrix = glm::translate(m_view_matrix, cursor_delta);
    });

    m_window.framebuffer_size.connect([this](int width, int height) {
        m_graph.set_size(width, height);
        m_plot.set_size(width, height);
    });

    m_graph.set_size(m_window.size());
    m_plot.set_size(m_window.size());
}

void AppContext::draw()
{
    for (auto &time_series : m_ts)
    {
        if (time_series.visible)
        {
            m_plot.draw(m_view_matrix,
                        *(time_series.ts),
                        m_plot_width,
                        time_series.colour,
                        time_series.y_offset,
                        m_show_line_segments);
        }
    }
    m_graph.draw_decorations(m_view_matrix);

    draw_gui();
}

void AppContext::draw_gui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImVec2 menubar_size;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close"))
            {
                m_window.request_close();
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

        // auto cursor_gs = m_graph->cursor_graphspace(_vp_matrix);
        // ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);
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

void AppContext::update_multisampling() const
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

void AppContext::update_vsync() const
{
    glfwSwapInterval(m_enable_vsync ? 1 : 0);
}

void AppContext::update_bgcolour() const
{
    m_window.set_bg_colour(m_bgcolor);
}

glm::dvec2 AppContext::screen2graph(const glm::ivec2 &value) const
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
std::pair<double, const char *> AppContext::human_readable(std::size_t size,
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
