#pragma once

#include "shader_utils.hpp"
#include "view.hpp"

class Axis : public View
{
  public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    Axis();
    ~Axis();

    void draw(const Window &window) const override;

    glm::dvec2 position() const override;
    void set_position(const glm::dvec2 &position) override;

    glm::dvec2 size() const override;
    void set_size(const glm::dvec2 &size) override;

    void set_orientation(Orientation ori);

    void on_scroll(double x, double y) override;

  private:
    glm::dvec2 m_position = glm::dvec2(0.0);
    glm::dvec2 m_size = glm::dvec2(100.0);

    unsigned int m_linebuf_vao;
    unsigned int m_linebuf_vbo;
    Program m_lines_shader;
    Orientation m_orientation;
};
