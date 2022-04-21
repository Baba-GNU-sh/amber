#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "timeseries.hpp"
#include "window.hpp"

/**
 * @brief Routines and buffers for drawing makers - verical lines with draggable handles.
 */
class MarkerRendererOpenGL
{
  public:
    MarkerRendererOpenGL(Window &window);
    ~MarkerRendererOpenGL();
    void draw(const std::string &label, int position_px, int gutter_size_px, const glm::vec3 &colour) const;

  private:
    Window &m_window;
    unsigned int m_handle_texture;
    unsigned int m_handle_vertex_buffer;
    unsigned int m_handle_vao;
    Program m_shader_program;
};
