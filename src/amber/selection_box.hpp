#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "window.hpp"
#include "view.hpp"

namespace amber
{
class SelectionBox : public View
{
  public:
    SelectionBox(Window &window);
    ~SelectionBox();
    SelectionBox(const SelectionBox &) = delete;
    SelectionBox &operator=(const SelectionBox &) = delete;
    SelectionBox(SelectionBox &&) = delete;
    SelectionBox &operator=(SelectionBox &&) = delete;

    void set_position(const glm::dvec2 &position) override;
    void set_size(const glm::dvec2 &size) override;
    void set_visible(bool visible);
    void draw() override;

  private:
    struct BoxVerticies
    {
        glm::vec2 vert[4];
    };

    Window &m_window;
    unsigned int m_vbo;
    unsigned int m_vao;
    Program m_shader;
    glm::dvec2 m_position;
    glm::dvec2 m_size;
    glm::vec3 m_colour;
    bool m_is_visible = false;
};
} // namespace amber
