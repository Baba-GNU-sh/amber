#pragma once

#include <algorithm>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <utils/transform.hpp>
#include "view.hpp"
#include "window.hpp"

class Window_GLFW : public Window, public View
{
  public:
    Window_GLFW(int width, int height, const std::string &title);
    virtual ~Window_GLFW();
    Window_GLFW(const Window_GLFW &) = delete;
    Window_GLFW &operator=(const Window_GLFW &) = delete;
    Window_GLFW(Window_GLFW &&) = delete;
    Window_GLFW &operator=(Window_GLFW &&) = delete;

    void init();

    void use() const;
    void finish() const;
    const Transform<double> &viewport_transform() const override;
    glm::dvec2 size() const override;
    GLFWwindow *handle();
    bool should_close() const override;
    void request_close() override;
    glm::dvec2 cursor() const override;
    void set_bg_colour(const glm::vec3 &col) override;
    glm::vec3 bg_colour();
    void set_fullscreen(bool enable) override;
    bool is_fullscreen() const override;
    glm::vec2 scaling() const;
    void scissor(int x, int y, int width, int height) const override;
    glm::ivec2 window_size() const override;

    virtual void render() const
    {
        use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw(*this);
        finish();
    }

  protected:
    GLFWwindow *m_window;
    virtual void handle_framebuffer_size_callback(int width, int height);
    virtual void handle_cursor_pos_callback(double xpos, double ypos);
    virtual void handle_scroll_callback(double xoffset, double yoffset);
    virtual void handle_mouse_button_callback(int button, int action, int mods);
    virtual void handle_key_callback(int key, int scancode, int action, int mods);

  private:
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void error_callback(int error, const char *msg);
    void update_vp_matrix();
    GLFWmonitor *get_current_monitor() const;

    std::string m_title;
    glm::vec3 m_bg_colour;
    bool m_fullscreen_mode;
    Transform<double> m_viewport_transform;
    std::shared_ptr<spdlog::logger> m_logger;
    glm::ivec2 m_windowed_size;
    glm::ivec2 m_windowed_pos;
    static bool m_first_window;
};
