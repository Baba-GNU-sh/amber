#pragma once

#include <glm/glm.hpp>
#include <database/timeseries.hpp>
#include "shader_utils.hpp"
#include "label.hpp"
#include "window.hpp"
#include "sprite.hpp"

class Marker : public View
{
  public:
    Marker(Window &window);
    ~Marker();
    Marker(const Marker &) = delete;
    Marker &operator=(const Marker &) = delete;
    Marker(Marker &&) = delete;
    Marker &operator=(Marker &&) = delete;

    void set_x_position(double position);
    void set_colour(const glm::vec3 &colour);
    void set_height(int height);
    void set_label_text(const std::string &text);
    void draw(const Window &) const override;

    glm::dvec2 position() const override;
    glm::dvec2 size() const override;

    bool is_dragging = false; // TODO The marker object should listen to mouse events from the
                              // window and decide when it is clicked and when to start dragging

  private:
    struct TextureCoord
    {
        glm::vec2 vertex_pos;
        glm::vec2 texture_pos;
    };

    void update_layout();

    Window &m_window;
    Sprite m_handle;
    Font m_font;
    Label m_label;
    unsigned int m_line_vertex_buffer;
    unsigned int m_line_vao;
    Program m_line_shader;
    double m_position;
    glm::vec3 m_colour;
    int m_height;
};
