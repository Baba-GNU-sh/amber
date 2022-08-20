#pragma once

#include <database/database.hpp>
#include <imgui.h>
#include "view.hpp"
#include "window_glfw.hpp"
#include "plugin_manager.hpp"
#include "window.hpp"
#include "graph.hpp"

class ImGuiMenuView : public View
{
  public:
    ImGuiMenuView(Window_GLFW &window,
                  PluginManager &plugin_manager,
                  Graph &graph,
                  Database &database,
                  GraphState &graph_state);

  private:
    void draw() override;
    void update_vsync() const;
    void update_call_glfinish();
    void update_multisampling();
    void update_bg_colour();
    void update_follow_latest_data();

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
    std::pair<double, const char *> static human_readable(std::size_t size,
                                                          double divisor = 1000,
                                                          std::vector<const char *> suffixes = {
                                                              "K", "M", "B", "T"});

    bool m_enable_vsync = true;
    bool m_call_glfinish = false;
    bool m_enable_multisampling = true;
    glm::vec3 m_clear_colour = glm::vec3(0.1, 0.1, 0.1);
    bool m_follow_latest_data = false;

    Window_GLFW &m_window;
    PluginManager &m_plugin_manager;
    Graph &m_graph;
    Database &m_database;
    GraphState &m_graph_state;
};