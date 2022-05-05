#include "window.hpp"
#include <GL/gl.h>
#include <GLFW/glfw3.h>

Window::Window(int width, int height, const std::string &title)
    : m_title(title), m_bg_colour(0.0), m_fullscreen_mode(false)
{
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    m_window = glfwCreateWindow(width, height, m_title.c_str(), NULL, NULL);
    if (!m_window)
    {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, Window::framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, Window::cursor_pos_callback);
    glfwSetScrollCallback(m_window, Window::scroll_callback);
    glfwSetMouseButtonCallback(m_window, Window::mouse_button_callback);
    glfwSetKeyCallback(m_window, Window::key_callback);

    update_vp_matrix(width, height);

    m_logger = spdlog::stdout_color_mt("Window(" + m_title + ")");
    m_logger->info("Initialized");
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
}

void Window::use() const
{
    glfwMakeContextCurrent(m_window);
    glClearColor(m_bg_colour.r, m_bg_colour.g, m_bg_colour.b, 1.0f);
}

void Window::finish() const
{
    glfwSwapBuffers(m_window);
}

const glm::mat3 &Window::vp_matrix() const
{
    return m_vp_matrix;
}

const glm::mat3 &Window::vp_matrix_inv() const
{
    return m_vp_matrix_inv;
}

glm::ivec2 Window::size() const
{
    glm::ivec2 size(0.0);
    glfwGetWindowSize(m_window, &size.x, &size.y);
    return size;
}

GLFWwindow *Window::handle()
{
    return m_window;
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::request_close()
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

glm::dvec2 Window::cursor() const
{
    glm::dvec2 ret;
    glfwGetCursorPos(m_window, &ret.x, &ret.y);
    return ret;
}

void Window::set_bg_colour(const glm::vec3 &col)
{
    m_bg_colour = col;
}

glm::vec3 Window::bg_colour()
{
    return m_bg_colour;
}

void Window::set_fullscreen(bool enable)
{
    m_fullscreen_mode = enable;

    if (m_fullscreen_mode)
    {
        // Backup the windowed size and position
        glfwGetWindowSize(m_window, &m_windowed_size.x, &m_windowed_size.y);
        glfwGetWindowPos(m_window, &m_windowed_pos.x, &m_windowed_pos.y);

        auto *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        glfwSetWindowMonitor(m_window,
                             NULL,
                             m_windowed_pos.x,
                             m_windowed_pos.y,
                             m_windowed_size.x,
                             m_windowed_size.y,
                             GLFW_DONT_CARE);
    }
}

bool Window::is_fullscreen()
{
    return m_fullscreen_mode;
}

boost::signals2::connection Window::on_resize(
    const resize_signal_t::slot_type &subscriber)
{
    return resize.connect(subscriber);
}

boost::signals2::connection Window::on_cursor_move(
    const cursor_move_signal_t::slot_type &subscriber)
{
    return cursor_move.connect(subscriber);
}

boost::signals2::connection Window::on_scroll(
    const scroll_signal_t::slot_type &subscriber)
{
    return scroll.connect(subscriber);
}

boost::signals2::connection Window::on_mouse_button(
    const mouse_button_signal_t::slot_type &subscriber)
{
    return mouse_button.connect(subscriber);
}

boost::signals2::connection Window::on_key(
    const key_signal_t::slot_type &subscriber)
{
    return key.connect(subscriber);
}

void Window::handle_framebuffer_size_callback(int width, int height)
{
    update_vp_matrix(width, height);
    glfwMakeContextCurrent(m_window);
    glViewport(0, 0, width, height);
    resize(width, height);
    m_logger->info("Window resized: {}x{}px", width, height);
}

void Window::handle_cursor_pos_callback(double xpos, double ypos)
{
    cursor_move(xpos, ypos);
}

void Window::handle_scroll_callback(double xoffset, double yoffset)
{
    scroll(xoffset, yoffset);
}

void Window::handle_mouse_button_callback(int button, int action, int mods)
{
    mouse_button(button, action, mods);
}

void Window::handle_key_callback(int key, int scancode, int action, int mods)
{
    this->key(key, scancode, action, mods);
}

void Window::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    win->handle_framebuffer_size_callback(width, height);
}

void Window::cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    win->handle_cursor_pos_callback(xpos, ypos);
}

void Window::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    win->handle_scroll_callback(xoffset, yoffset);
}

void Window::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    win->handle_mouse_button_callback(button, action, mods);
}

void Window::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
    win->handle_key_callback(key, scancode, action, mods);
}

void Window::update_vp_matrix(int width, int height)
{
    const glm::mat3 identity(1.0f);
    auto vp_matrix = glm::scale(identity, glm::vec2(width / 2, -height / 2));
    m_vp_matrix = glm::translate(vp_matrix, glm::vec2(1, -1));
    m_vp_matrix_inv = glm::inverse(m_vp_matrix);
}
