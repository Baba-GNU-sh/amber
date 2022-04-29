#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "text_renderer_opengl.hpp"
#include <database/timeseries.hpp>
#include "window.hpp"
#include "sprite.hpp"

/**
 * @brief Routines and buffers for drawing makers - which are vertical lines and a little handle for
 * dragging.
 */
class Marker
{
  public:
    enum class MarkerStyle
    {
        Center,
        Left,
        Right
    };

    Marker(Window &window);
    ~Marker();
    void set_position(const glm::ivec2 &position);
    void set_colour(const glm::vec3 &colour);
    void set_height(int height);
    void draw() const;

  private:
    unsigned int load_texture(const std::string &filename) const;

    Window &m_window;
    Sprite m_handle;
    unsigned int m_line_vertex_buffer;
    unsigned int m_line_vao;
    Program m_line_shader;
    glm::ivec2 m_position;
    glm::vec3 m_colour;
    int m_height;
};
