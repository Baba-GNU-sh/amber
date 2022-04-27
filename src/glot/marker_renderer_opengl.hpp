#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "text_renderer_opengl.hpp"
#include <database/timeseries.hpp>
#include "window.hpp"

/**
 * @brief Routines and buffers for drawing makers - which are vertical lines and a little handle for
 * dragging.
 */
class MarkerRendererOpenGL
{
  public:
    enum class MarkerStyle
    {
        Center,
        Left,
        Right
    };
    MarkerRendererOpenGL(Window &window);
    ~MarkerRendererOpenGL();
    void draw(int position_px,
              int gutter_size_px,
              const glm::vec3 &colour,
              MarkerStyle style) const;

  private:
    unsigned int load_texture(const std::string &filename) const;

    Window &m_window;
    unsigned int m_handle_texture_center;
    unsigned int m_handle_texture_left;
    unsigned int m_handle_texture_right;
    unsigned int m_handle_vertex_buffer;
    unsigned int m_handle_vao;
    unsigned int m_line_vertex_buffer;
    unsigned int m_line_vao;
    Program m_sprite_shader;
    Program m_line_shader;
};
