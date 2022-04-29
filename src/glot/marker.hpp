#pragma once

#include <glm/glm.hpp>
#include <database/timeseries.hpp>
#include "shader_utils.hpp"
#include "label.hpp"
#include "window.hpp"
#include "sprite.hpp"

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
    void set_style(MarkerStyle style);
    void set_label_text(const std::string &text);
    void draw() const;

    bool is_dragging = false; // TODO make this better

  private:
    struct TextureCoord
    {
        glm::vec2 vertex_pos;
        glm::vec2 texture_pos;
    };

    unsigned int load_texture(const std::string &filename) const;

    Window &m_window;
    Sprite m_handle;
    Font m_font;
    Label m_label;
    unsigned int m_line_vertex_buffer;
    unsigned int m_line_vao;
    Program m_line_shader;
    glm::ivec2 m_position;
    glm::vec3 m_colour;
    int m_height;
};
