#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui_window.hpp"
#include "sprite.hpp"
#include "label.hpp"
#include "axis.hpp"

class Panel : public View
{
    void draw(const Window &window) const override
    {
        ImGui::Begin("Help");
        ImGui::Text("%.1f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }
};

int main()
{
    ImGuiContextWindow window(1024, 768, "demo");

    auto panel = std::make_shared<Panel>();
    window.add_imgui_view(panel);

    int offset = 0;
    for (int i = 0; i < 10; i++)
    {
        auto sprite = std::make_shared<Sprite>("marker_center.png");
        sprite->set_position(glm::dvec2(offset, 0));
        offset += sprite->size().x;
        window.add(sprite);
    }

    Font font("proggy_clean.png");

    offset = 0;
    for (int i = 0; i < 10; i++)
    {
        auto label = std::make_shared<Label>(font, "Hello!");
        window.add(label);
        label->set_position(glm::dvec2(offset, 50));
        label->set_colour(glm::vec3(1.0, 1.0, 1.0));
        offset += label->size().x;
    }

    Transform<double> graph_transform;
    auto axis_vertical = std::make_shared<Axis>(Axis::Orientation::Vertical, window);
    axis_vertical->set_position(glm::dvec2(0, 100));
    axis_vertical->set_size(glm::dvec2(100, 500));
    window.add(axis_vertical);

    auto axis_horizontal = std::make_shared<Axis>(Axis::Orientation::Horizontal, window);
    axis_horizontal->set_position(glm::dvec2(100, 600));
    axis_horizontal->set_size(glm::dvec2(500, 100));
    window.add(axis_horizontal);

    while (!window.should_close())
    {
        glfwPollEvents();
        window.render();
        graph_transform.scale(glm::dvec2(1.001));
    }
}
