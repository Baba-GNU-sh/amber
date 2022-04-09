#include "app_context.hpp"

#include <GLFW/glfw3.h>
#include "graph.hpp"
#include "database.hpp"
#include "plugin_context.hpp"
#include "plugin_manager.hpp"
#include "window.hpp"
#include <imgui.h>
#include <unistd.h>
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include "imgui_window.hpp"
#include "app_context.hpp"


AppContext::AppContext(Database &database,
                       Graph &graph,
                       Window &window,
                       PluginManager &plugin_manager)
    : m_database(database), m_graph(graph), m_window(window), m_plugin_manager(plugin_manager)
{
    std::vector<glm::vec3> plot_colours = {
        glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};

    const auto &data = m_database.data();
    m_ts.resize(data.size());
    auto col = plot_colours.begin();
    std::transform(data.begin(), data.end(), m_ts.begin(), [&col, &plot_colours](const auto &ts) {
        TimeSeriesContainer cont;
        cont.name = ts.first;
        cont.ts = ts.second;
        cont.colour = *col++;
        cont.visible = true;
        cont.y_offset = 0.0f;
        if (col == plot_colours.end())
        {
            col = plot_colours.begin();
        }
        return cont;
    });

    m_bgcolor = glm::vec3(0.1, 0.1, 0.1);
    _update_multisampling();
    _update_vsync();
    _update_bgcolour();
}

void AppContext::draw()
{
    for (auto &time_series : m_ts)
    {
        if (time_series.visible)
        {
            const TimeSeries &tsref = *(time_series.ts);
            m_graph.draw_plot(m_plot_width, time_series.colour, time_series.y_offset, tsref);
        }
    }
    m_graph.draw_decorations();

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

        // auto viewmat = m_graph->view_matrix();
        // ImGui::Text("View Matrix:");
        // for (int i = 0; i < 3; i++)
        // {
        //     ImGui::Text("%f %f %f", viewmat[0][i], viewmat[1][i], viewmat[2][i]);
        // }

        // auto cursor_gs = m_graph->cursor_graphspace(_vp_matrix);
        // ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);
    }

    if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Checkbox("Enable VSync", &m_enable_vsync))
        {
            _update_vsync();
        }

        if (ImGui::Checkbox("Multisampling", &m_enable_multisampling))
        {
            _update_multisampling();
        }

        if (ImGui::ColorEdit3("BG Colour", &(m_bgcolor.x)))
        {
            _update_bgcolour();
        }

        ImGui::SliderInt("Line Width", &m_plot_width, 1, 5);
    }

    if (ImGui::CollapsingHeader("Plots", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto &plugin : m_ts)
        {
            // Im ImGui, widgets need unique label names
            // Anything after the "##" is not displayed
            const auto label_name = "##" + plugin.name;
            ImGui::Checkbox(label_name.c_str(), &(plugin.visible));
            ImGui::SameLine();
            ImGui::ColorEdit3(
                plugin.name.c_str(), &(plugin.colour.x), ImGuiColorEditFlags_NoInputs);
            const auto slider_name = "Y Offset##" + plugin.name;
            ImGui::DragFloat(slider_name.c_str(), &(plugin.y_offset), 0.01);
        }
    }

    m_plugin_manager.draw_dialogs();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppContext::_update_multisampling()
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

void AppContext::_update_vsync() const
{
    glfwSwapInterval(m_enable_vsync ? 1 : 0);
}

void AppContext::_update_bgcolour() const
{
    glClearColor(m_bgcolor.r, m_bgcolor.g, m_bgcolor.b, 1.0f);
}
