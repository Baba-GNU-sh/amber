#include "window_glfw_imgui.hpp"
#include "window_glfw.hpp"

Window_GLFW_ImGui::Window_GLFW_ImGui(int width, int height, const std::string &title)
    : Window_GLFW(width, height, title)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

Window_GLFW_ImGui::~Window_GLFW_ImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Window_GLFW_ImGui::add_imgui_view(View *view)
{
    m_imgui_views.push_back(view);
}

void Window_GLFW_ImGui::render() const
{
    use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    std::for_each(m_views.begin(), m_views.end(), [this](const auto &view) { view->draw(*this); });

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    std::for_each(m_imgui_views.begin(), m_imgui_views.end(), [this](const auto &view) {
        view->draw(*this);
    });

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    finish();
}

void Window_GLFW_ImGui::handle_cursor_pos_callback(double xpos, double ypos)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        Window_GLFW::handle_cursor_pos_callback(xpos, ypos);
    }
}

void Window_GLFW_ImGui::handle_scroll_callback(double xoffset, double yoffset)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        Window_GLFW::handle_scroll_callback(xoffset, yoffset);
    }
}

void Window_GLFW_ImGui::handle_mouse_button_callback(int button, int action, int mods)
{
    if (!ImGui::GetIO().WantCaptureMouse)
    {
        Window_GLFW::handle_mouse_button_callback(button, action, mods);
    }
}
