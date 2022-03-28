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

class Window
{
  public:
    Window() : _bgcolour(0.2f, 0.2f, 0.2f)
    {
        m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLot", NULL, NULL);
        if (!m_window)
        {
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Register callbacks
        glfwSetWindowUserPointer(m_window, this);
        glfwMakeContextCurrent(m_window);
        glfwSetFramebufferSizeCallback(m_window, Window::framebuffer_size_callback);
        glfwSetCursorPosCallback(m_window, Window::cursor_pos_callback);
        glfwSetScrollCallback(m_window, Window::scroll_callback);
        glfwSetMouseButtonCallback(m_window, Window::mouse_button_callback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glfwDestroyWindow(m_window);
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Our graph requires depth testing to be enabled in order to render correcly
        glEnable(GL_DEPTH_TEST);

        // Blending should be enabled for text to be rendered correclty
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        // Update settings
        _update_multisampling();
        _update_vsync();
        _update_bgcolour();

        m_graph = std::make_shared<GraphView>();
        m_graph->set_size(SCR_WIDTH, SCR_HEIGHT);

        update_viewport_matrix(SCR_WIDTH, SCR_HEIGHT);
    }

    ~Window()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(m_window);
    }

    void spin()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            m_graph->draw();
            render_imgui();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(m_window);
        }
    }

  private:
    void _update_multisampling()
    {
        if (_enable_multisampling)
        {
            glEnable(GL_MULTISAMPLE);
        }
        else
        {
            glDisable(GL_MULTISAMPLE);
        }
    }

    void _update_vsync() const
    {
        glfwSwapInterval(_enable_vsync ? 1 : 0);
    }

    void _update_bgcolour() const
    {
        glClearColor(_bgcolour.r, _bgcolour.g, _bgcolour.b, 1.0f);
    }

    void update_viewport_matrix(int width, int height)
    {
        const glm::mat3 identity(1.0f);
        auto vp_matrix = glm::scale(identity, glm::vec2(width / 2, -height / 2));
        vp_matrix = glm::translate(vp_matrix, glm::vec2(1, -1));
        m_graph->update_viewport_matrix(vp_matrix);
    }

    void render_imgui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Info");

        ImGui::Text("Use the right mouse button to pan");
        ImGui::Text("Use the scroll wheel to zoom");
        ImGui::Text("Try scrolling in one of the gutters");

        ImGui::Text("Perf: %.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        auto viewmat = m_graph->get_view_matrix();
        ImGui::Text("View Matrix:");
        for (int i = 0; i < 3; i++)
        {
            ImGui::Text("%f %f %f", viewmat[0][i], viewmat[1][i], viewmat[2][i]);
        }

        auto cursor_gs = m_graph->get_cursor_graphspace();
        ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);

        if (ImGui::Checkbox("Enable VSync", &_enable_vsync))
        {
            _update_vsync();
        }

        if (ImGui::Checkbox("Multisampling", &_enable_multisampling))
        {
            _update_multisampling();
        }

        ImGui::SliderInt("Plot Width", m_graph->get_plot_thickness(), 1, 8);

        auto *colour = m_graph->get_plot_colour();
        ImGui::ColorEdit3("Plot Colour", &(colour->x));

        colour = m_graph->get_minmax_colour();
        ImGui::ColorEdit3("MinMax Colour", &(colour->x));

        if (ImGui::ColorEdit3("BG Colour", &(_bgcolour.x)))
        {
            _update_bgcolour();
        }

        ImGui::End();

        ImGui::Render();
    }

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        spdlog::info("Window resized: {}x{}px", width, height);
        glViewport(0, 0, width, height);
        Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));

        // Update the graph to fill the screen
        win->m_graph->set_size(width, height);
        win->update_viewport_matrix(width, height);
    }

    static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
    {
        Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        win->m_graph->cursor_move(xpos, ypos);
    }

    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        win->m_graph->mouse_scroll(xoffset, yoffset);
    }

    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        ImGuiIO &io = ImGui::GetIO();
        bool capture = io.WantCaptureMouse;
        if (capture)
            return;

        Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        win->m_graph->mouse_button(button, action, mods);
    }

    static constexpr unsigned int SCR_WIDTH = 800;
    static constexpr unsigned int SCR_HEIGHT = 600;

    GLFWwindow *m_window;
    std::shared_ptr<GraphView> m_graph;

    bool _enable_vsync = true;
    bool _enable_multisampling = true;
    glm::vec3 _bgcolour;
};
