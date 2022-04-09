#pragma once

#include <string>
#include <imgui.h>
#include "window_container.hpp"
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

class ImGuiContextWindow : public WindowContainer
{
  public:
    ImGuiContextWindow(int width, int height, const std::string &title);
    virtual ~ImGuiContextWindow();

  private:
    virtual void handle_cursor_pos_callback(double xpos, double ypos) override;
    virtual void handle_scroll_callback(double xoffset, double yoffset) override;
    virtual void handle_mouse_button_callback(int button, int action, int mods) override;
};
