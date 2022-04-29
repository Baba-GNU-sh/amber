#pragma once

#include "graph_state.hpp"

class ImGuiDatabasePanel
{
  public:
    ImGuiDatabasePanel(GraphState &state);
    void draw_imgui_panel();

  private:
    GraphState &m_state;
};