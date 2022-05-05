#include "imgui_window.hpp"

ImGuiContextWindow::ImGuiContextWindow(int width, int height, const std::string &title)
    : Window(width, height, title)
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

void ImGuiContextWindow::handle_cursor_pos_callback(double xpos, double ypos)
{
    if (!ImGui::GetIO().WantCaptureMouse)
        cursor_move(xpos, ypos);
}

void ImGuiContextWindow::handle_scroll_callback(double xoffset, double yoffset)
{
    if (!ImGui::GetIO().WantCaptureMouse)
        scroll(xoffset, yoffset);
}

void ImGuiContextWindow::handle_mouse_button_callback(int button, int action, int mods)
{
    if (!ImGui::GetIO().WantCaptureMouse)
        mouse_button(button, action, mods);
}
