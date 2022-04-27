#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "window.hpp"

class LineRendererOpenGL
{
  public:
    LineRendererOpenGL(Window &window);
    ~LineRendererOpenGL();
    void draw_line(const glm::dvec2 &start, const glm::dvec2 &end, const glm::vec3 &colour) const;

  private:
    Window &m_window;
    GLuint m_linebuf_vao;
    GLuint m_line_vertex_buffer;
    Program m_line_shader;
};
