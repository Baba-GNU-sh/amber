#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <random>

#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include <imgui.h>

#include "shader_utils.hpp"

#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>

#include "graph.hpp"

#include "spdlog/spdlog.h"

#include "database.hpp"
#include "plugin_manager.hpp"

class Window
{
  public:
    Window(const Database &db, PluginManager &plugins);
    ~Window();
    void spin();

  private:
    void _update_multisampling();
    void _update_vsync() const;
    void _update_bgcolour() const;
    void update_viewport_matrix(int width, int height);
    void render_imgui();
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    static constexpr unsigned int SCR_WIDTH = 800;
    static constexpr unsigned int SCR_HEIGHT = 600;

    glm::vec3 _bgcolour;
    glm::ivec2 _win_size;
    const Database &_database;
    PluginManager &_plugin_manager;
    bool _enable_vsync = true;
    bool _enable_multisampling = true;

    GLFWwindow *m_window;
    std::shared_ptr<GraphView> m_graph;
};
