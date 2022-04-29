#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "window.hpp"

class SelectionBox
{
  public:
    SelectionBox(Window &window);
    ~SelectionBox();
    SelectionBox(const SelectionBox &) = delete;
    SelectionBox &operator=(const SelectionBox &) = delete;
    SelectionBox(SelectionBox &&) = delete;
    SelectionBox &operator=(SelectionBox &&) = delete;

    void set_position(const glm::ivec2 &position);
    void set_size(const glm::ivec2 &size);
    void draw() const;

  private:
    struct BoxVerticies
    {
        glm::vec2 vert[4];
    };

    Window &m_window;
    unsigned int m_vbo;
    unsigned int m_vao;
    Program m_shader;
    glm::ivec2 m_position;
    glm::ivec2 m_size;
    glm::vec3 m_colour;
};
