#include <imgui.h>
#include "imgui_database_panel.hpp"

ImGuiDatabasePanel::ImGuiDatabasePanel(GraphState &state) : m_state(state)
{
    //
}

void ImGuiDatabasePanel::draw_imgui_panel()
{
    for (auto &plugin : m_state.timeseries)
    {
        // Im ImGui, widgets need unique label names
        // Anything after the "##" is counted towards the uniqueness but is not displayed
        const auto label_name = "##" + plugin.name;
        ImGui::Checkbox(label_name.c_str(), &(plugin.visible));
        ImGui::SameLine();
        ImGui::ColorEdit3(plugin.name.c_str(), &(plugin.colour.x), ImGuiColorEditFlags_NoInputs);
        const auto slider_name = "Y offset##" + plugin.name;
        ImGui::DragFloat(slider_name.c_str(), &(plugin.y_offset), 0.01);
    }
}
