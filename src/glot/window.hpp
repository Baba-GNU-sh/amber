#pragma once

#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <boost/signals2.hpp>

class Window
{
  public:
    Window(int width, int height, const std::string &title);
    virtual ~Window();
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

    void use() const;
    void finish() const;
    const glm::mat3 &vp_matrix() const;
    const glm::mat3 &vp_matrix_inv() const;
    glm::ivec2 size() const;
    GLFWwindow *handle();
    bool should_close() const;
    void request_close();
    glm::dvec2 cursor() const;
    void set_bg_colour(const glm::vec3 &col);
    glm::vec3 bg_colour();
    void set_fullscreen(bool enable);
    bool is_fullscreen();
    glm::vec2 scaling() const;
    void scissor(int x, int y, int width, int height) const;

    typedef boost::signals2::signal<void(int, int)> resize_signal_t;
    typedef boost::signals2::signal<void(double, double)> cursor_move_signal_t;
    typedef boost::signals2::signal<void(double, double)> scroll_signal_t;
    typedef boost::signals2::signal<void(int, int, int)> mouse_button_signal_t;
    typedef boost::signals2::signal<void(int, int, int, int)> key_signal_t;

    boost::signals2::connection on_resize(const resize_signal_t::slot_type &subscriber);
    boost::signals2::connection on_cursor_move(const cursor_move_signal_t::slot_type &subscriber);
    boost::signals2::connection on_scroll(const scroll_signal_t::slot_type &subscriber);
    boost::signals2::connection on_mouse_button(const mouse_button_signal_t::slot_type &subscriber);
    boost::signals2::connection on_key(const key_signal_t::slot_type &subscriber);

  protected:
    GLFWwindow *m_window;
    virtual void handle_framebuffer_size_callback(int width, int height);
    virtual void handle_cursor_pos_callback(double xpos, double ypos);
    virtual void handle_scroll_callback(double xoffset, double yoffset);
    virtual void handle_mouse_button_callback(int button, int action, int mods);
    virtual void handle_key_callback(int key, int scancode, int action, int mods);

    resize_signal_t resize;
    cursor_move_signal_t cursor_move;
    scroll_signal_t scroll;
    mouse_button_signal_t mouse_button;
    key_signal_t key;

  private:
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void update_vp_matrix();

    std::string m_title;
    glm::vec3 m_bg_colour;
    bool m_fullscreen_mode;
    glm::mat3 m_vp_matrix;
    glm::mat3 m_vp_matrix_inv;
    std::shared_ptr<spdlog::logger> m_logger;
    glm::ivec2 m_windowed_size;
    glm::ivec2 m_windowed_pos;
};
