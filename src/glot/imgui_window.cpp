#include "imgui_window.hpp"
#include "window_glfw.hpp"

ImGuiContextWindow::ImGuiContextWindow(int width, int height, const std::string &title)
    : Window_GLFW(width, height, title)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

ImGuiContextWindow::~ImGuiContextWindow()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiContextWindow::add_imgui_view(std::shared_ptr<View> view)
{
    m_imgui_views.push_back(view);
}

void ImGuiContextWindow::render() const
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

void ImGuiContextWindow::handle_cursor_pos_callback(double xpos, double ypos)
{
    if (!ImGui::GetIO().WantCaptureMouse)
        Window_GLFW::handle_cursor_pos_callback(xpos, ypos);
}

void ImGuiContextWindow::handle_scroll_callback(double xoffset, double yoffset)
{
    if (!ImGui::GetIO().WantCaptureMouse)
        Window_GLFW::handle_scroll_callback(xoffset, yoffset);
}

void ImGuiContextWindow::handle_mouse_button_callback(int button, int action, int mods)
{
    if (!ImGui::GetIO().WantCaptureMouse)
        Window_GLFW::handle_mouse_button_callback(button, action, mods);
}
