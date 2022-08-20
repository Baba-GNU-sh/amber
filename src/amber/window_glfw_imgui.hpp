#pragma once

#include <string>
#include <imgui.h>
#include "window_glfw.hpp"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

class Window_GLFW_ImGui : public Window_GLFW
{
  public:
    Window_GLFW_ImGui(int width, int height, const std::string &title);
    virtual ~Window_GLFW_ImGui();
    void add_imgui_view(View *view);
    void render() override;

  private:
    virtual void handle_cursor_pos_callback(double xpos, double ypos) override;
    virtual void handle_scroll_callback(double xoffset, double yoffset) override;
    virtual void handle_mouse_button_callback(int button, int action, int mods) override;

    std::vector<View *> m_imgui_views;
};
