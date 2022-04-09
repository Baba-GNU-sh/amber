#include "window_container.hpp"

WindowContainer::WindowContainer(int width, int height, const std::string &title)
{
    m_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!m_window)
    {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, WindowContainer::framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, WindowContainer::cursor_pos_callback);
    glfwSetScrollCallback(m_window, WindowContainer::scroll_callback);
    glfwSetMouseButtonCallback(m_window, WindowContainer::mouse_button_callback);
    glfwSetKeyCallback(m_window, WindowContainer::key_callback);

    update_vp_matrix(width, height);

    m_logger = spdlog::stdout_color_mt("Window::" + std::string(title));
    m_logger->info("Initialized");
}

WindowContainer::~WindowContainer()
{
    glfwDestroyWindow(m_window);
}

void WindowContainer::use() const
{
    glfwMakeContextCurrent(m_window);
}

void WindowContainer::finish() const
{
    glfwSwapBuffers(m_window);
}

const glm::mat3 &WindowContainer::vp_matrix() const
{
    return m_vp_matrix;
}

const glm::mat3 &WindowContainer::vp_matrix_inv() const
{
    return m_vp_matrix_inv;
}

const glm::ivec2 &WindowContainer::size() const
{
    return m_size;
}

GLFWwindow *WindowContainer::handle()
{
    return m_window;
}

bool WindowContainer::should_close() const
{
    return glfwWindowShouldClose(m_window);
}

void WindowContainer::request_close()
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void WindowContainer::handle_framebuffer_size_callback(int width, int height)
{
    update_vp_matrix(width, height);
    glfwMakeContextCurrent(m_window);
    glViewport(0, 0, width, height);
    framebuffer_size(width, height);
    m_logger->info("Window resized: {}x{}px", width, height);
}

void WindowContainer::handle_cursor_pos_callback(double xpos, double ypos)
{
    cursor_pos(xpos, ypos);
}

void WindowContainer::handle_scroll_callback(double xoffset, double yoffset)
{
    scroll(xoffset, yoffset);
}

void WindowContainer::handle_mouse_button_callback(int button, int action, int mods)
{
    mouse_button(button, action, mods);
}

void WindowContainer::handle_key_callback(int key, int scancode, int action, int mods)
{
    this->key(key, scancode, action, mods);
}

void WindowContainer::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    auto *win = static_cast<WindowContainer *>(glfwGetWindowUserPointer(window));
    win->handle_framebuffer_size_callback(width, height);
}

void WindowContainer::cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    // if (ImGui::GetIO().WantCaptureMouse)
    //     return;

    auto *win = static_cast<WindowContainer *>(glfwGetWindowUserPointer(window));
    win->handle_cursor_pos_callback(xpos, ypos);
}

void WindowContainer::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // if (ImGui::GetIO().WantCaptureMouse)
    //     return;

    auto *win = static_cast<WindowContainer *>(glfwGetWindowUserPointer(window));
    win->handle_scroll_callback(xoffset, yoffset);
}

void WindowContainer::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // if (ImGui::GetIO().WantCaptureMouse)
    //     return;

    auto *win = static_cast<WindowContainer *>(glfwGetWindowUserPointer(window));
    win->handle_mouse_button_callback(button, action, mods);
}

void WindowContainer::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    auto *win = static_cast<WindowContainer *>(glfwGetWindowUserPointer(window));
    win->handle_key_callback(key, scancode, action, mods);
}

void WindowContainer::update_vp_matrix(int width, int height)
{
    const glm::mat3 identity(1.0f);
    auto vp_matrix = glm::scale(identity, glm::vec2(width / 2, -height / 2));
    m_vp_matrix = glm::translate(vp_matrix, glm::vec2(1, -1));
    m_vp_matrix_inv = glm::inverse(m_vp_matrix);
    m_size = glm::ivec2(width, height);
}
