#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "window_glfw_imgui.hpp"
#include "sprite.hpp"
#include "label.hpp"
#include "axis.hpp"

class Panel : public View
{
    void draw(const Window &window) override
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
    Window_GLFW_ImGui window(1024, 768, "demo");

    Panel panel;
    window.add_imgui_view(&panel);

    int offset = 0;
    for (int i = 0; i < 10; i++)
    {
        auto sprite = new Sprite("marker_center.png");
        sprite->set_position(glm::dvec2(offset, 0));
        offset += sprite->size().x;
        window.add_view(sprite);
    }

    Font font("proggy_clean.png");

    offset = 0;
    for (int i = 0; i < 10; i++)
    {
        auto label = new Label(font, "Hello!");
        window.add_view(label);
        label->set_position(glm::dvec2(offset, 50));
        label->set_colour(glm::vec3(1.0, 1.0, 1.0));
        offset += label->size().x;
    }

    Axis<AxisVertical> axis_vertical(window);
    axis_vertical.set_position(glm::dvec2(0, 100));
    axis_vertical.set_size(glm::dvec2(100, 500));
    window.add_view(&axis_vertical);

    Axis<AxisVertical> axis_horizontal(window);
    axis_vertical.set_position(glm::dvec2(0, 100));
    axis_vertical.set_size(glm::dvec2(100, 500));
    window.add_view(&axis_horizontal);

    Transform<double> graph_transform;
    while (!window.should_close())
    {
        glfwPollEvents();
        window.render();
        graph_transform.scale(glm::dvec2(1.001));
        axis_vertical.set_graph_transform(graph_transform);
        axis_horizontal.set_graph_transform(graph_transform);
    }
}
