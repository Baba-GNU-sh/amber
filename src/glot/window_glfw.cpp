#include "window_glfw.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "graph_utils.hpp"

bool Window_GLFW::m_first_window = true;

void Window_GLFW::init()
{
    const auto window_size = size();
    std::for_each(m_views.begin(), m_views.end(), [&window_size](const auto &view) {
        view->on_resize(window_size.x, window_size.y);
    });
}

Window_GLFW::Window_GLFW(int width, int height, const std::string &title)
    : m_title(title), m_bg_colour(0.0), m_fullscreen_mode(false)
{
    if (m_first_window)
    {
        glfwSetErrorCallback(Window_GLFW::error_callback);
        glfwInit();
    }

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
    glfwSetFramebufferSizeCallback(m_window, Window_GLFW::framebuffer_size_callback);
    glfwSetCursorPosCallback(m_window, Window_GLFW::cursor_pos_callback);
    glfwSetScrollCallback(m_window, Window_GLFW::scroll_callback);
    glfwSetMouseButtonCallback(m_window, Window_GLFW::mouse_button_callback);
    glfwSetKeyCallback(m_window, Window_GLFW::key_callback);

    update_vp_matrix();

    m_logger = spdlog::stdout_color_mt("Window_GLFW(" + m_title + ")");
    m_logger->info("Initialized");

    if (m_first_window)
    {
        m_logger->info("Loading OpenGL extenstions via GLAD");
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }
        m_first_window = false;
    }
}

Window_GLFW::~Window_GLFW()
{
    glfwDestroyWindow(m_window);
}

void Window_GLFW::use() const
{
    glfwMakeContextCurrent(m_window);
    glClearColor(m_bg_colour.r, m_bg_colour.g, m_bg_colour.b, 1.0f);
}

void Window_GLFW::set_call_glfinish(bool value)
{
    m_call_glfinish = value;
}

void Window_GLFW::finish() const
{
    glfwSwapBuffers(m_window);

    if (m_call_glfinish)
    {
        glFinish();
    }
}

const Transform<double> &Window_GLFW::viewport_transform() const
{
    return m_viewport_transform;
}

glm::dvec2 Window_GLFW::size() const
{
    glm::ivec2 size(0.0);
    glfwGetWindowSize(m_window, &size.x, &size.y);
    return size;
}

glm::ivec2 Window_GLFW::window_size() const
{
    glm::ivec2 size(0.0);
    glfwGetWindowSize(m_window, &size.x, &size.y);
    return size;
}

void Window_GLFW::render()
{
    use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw(*this);
    finish();
}

GLFWwindow *Window_GLFW::handle()
{
    return m_window;
}

bool Window_GLFW::should_close() const
{
    return glfwWindowShouldClose(m_window);
}

void Window_GLFW::request_close()
{
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

glm::dvec2 Window_GLFW::cursor() const
{
    glm::dvec2 ret;
    glfwGetCursorPos(m_window, &ret.x, &ret.y);
    return ret;
}

void Window_GLFW::set_bg_colour(const glm::vec3 &col)
{
    m_bg_colour = col;
}

glm::vec3 Window_GLFW::bg_colour()
{
    return m_bg_colour;
}

void Window_GLFW::set_fullscreen(bool enable)
{
    m_fullscreen_mode = enable;

    if (m_fullscreen_mode)
    {
        // Backup the windowed size and position
        glfwGetWindowSize(m_window, &m_windowed_size.x, &m_windowed_size.y);
        glfwGetWindowPos(m_window, &m_windowed_pos.x, &m_windowed_pos.y);

        auto *monitor = get_current_monitor();
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

bool Window_GLFW::is_fullscreen() const
{
    return m_fullscreen_mode;
}

glm::vec2 Window_GLFW::scaling() const
{
    float xscale, yscale;
    glfwGetWindowContentScale(m_window, &xscale, &yscale);
    return glm::vec2(xscale, yscale);
}

void Window_GLFW::scissor(int x, int y, int width, int height) const
{
    const auto window_size = size();
    const auto pos = glm::vec2(x, window_size.y - (y + height)) * scaling();
    const auto size = glm::vec2(width, height) * scaling();
    glScissor(pos.x, pos.y, size.x, size.y);
}

void Window_GLFW::handle_framebuffer_size_callback(int width, int height)
{
    // TODO: We probably don't want to bother passing on the framebuffer size...?
    const auto size_px = size();

    update_vp_matrix();
    glfwMakeContextCurrent(m_window);
    glViewport(0, 0, width, height);

    std::for_each(m_views.begin(), m_views.end(), [&size_px](const auto &view) {
        view->on_resize(size_px.x, size_px.y);
    });

    m_logger->info("Window_GLFW framebuffer resized: {}x{}px", width, height);
}

void Window_GLFW::handle_cursor_pos_callback(double xpos, double ypos)
{
    on_cursor_move(*this, xpos, ypos);
}

void Window_GLFW::handle_scroll_callback(double xoffset, double yoffset)
{
    on_scroll(*this, xoffset, yoffset);
}

void Window_GLFW::handle_mouse_button_callback(int button, int action, int mods)
{
    on_mouse_button(cursor(), button, action, mods);
}

void Window_GLFW::handle_key_callback(int key, int scancode, int action, int mods)
{
    on_key(*this, key, scancode, action, mods);
}

void Window_GLFW::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    auto *win = static_cast<Window_GLFW *>(glfwGetWindowUserPointer(window));
    win->handle_framebuffer_size_callback(width, height);
}

void Window_GLFW::cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
    auto *win = static_cast<Window_GLFW *>(glfwGetWindowUserPointer(window));
    win->handle_cursor_pos_callback(xpos, ypos);
}

void Window_GLFW::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto *win = static_cast<Window_GLFW *>(glfwGetWindowUserPointer(window));
    win->handle_scroll_callback(xoffset, yoffset);
}

void Window_GLFW::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    auto *win = static_cast<Window_GLFW *>(glfwGetWindowUserPointer(window));
    win->handle_mouse_button_callback(button, action, mods);
}

void Window_GLFW::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto *win = static_cast<Window_GLFW *>(glfwGetWindowUserPointer(window));
    win->handle_key_callback(key, scancode, action, mods);
}

void Window_GLFW::update_vp_matrix()
{
    const auto fb_size = size();
    const glm::mat3 identity(1.0f);
    auto vp_matrix = glm::scale(identity, glm::vec2(fb_size.x / 2, -fb_size.y / 2));
    vp_matrix = glm::translate(vp_matrix, glm::vec2(1, -1));
    m_viewport_transform.update(vp_matrix);
}

/**
 * @brief Get the monitor that the window is currently most "on".
 * See this stackoverflow answer:
 * https://stackoverflow.com/questions/21421074/how-to-create-a-full-screen-window-on-the-current-monitor-with-glfw
 */
GLFWmonitor *Window_GLFW::get_current_monitor() const
{
    int nmonitors, i;

    int mx, my, mw, mh;

    GLFWmonitor **monitors;
    const GLFWvidmode *mode;

    int bestoverlap = 0;
    GLFWmonitor *bestmonitor = nullptr;

    int wx, wy, ww, wh;
    glfwGetWindowPos(m_window, &wx, &wy);
    glfwGetWindowSize(m_window, &ww, &wh);
    monitors = glfwGetMonitors(&nmonitors);

    for (i = 0; i < nmonitors; i++)
    {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &mx, &my);
        mw = mode->width;
        mh = mode->height;

        int overlap = std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
                      std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

        if (bestoverlap < overlap)
        {
            bestoverlap = overlap;
            bestmonitor = monitors[i];
        }
    }

    return bestmonitor;
}

void Window_GLFW::error_callback(int error, const char *msg)
{
    spdlog::error("[{}] {}", error, msg);
}
