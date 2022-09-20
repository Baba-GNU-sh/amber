#pragma once

#include <sigslot/signal.hpp>

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

    double x_position() const;
    bool is_visible() const;
    void set_x_position(double position);
    void set_graph_transform(const Transform<double> &transform);
    void set_screen_height(int height);
    void set_colour(const glm::vec3 &colour);
    void set_label_text(const std::string &text);
    void set_visible(bool visible);
    void draw() override;
    Hitbox<double> hitbox() const override;

    glm::dvec2 position() const override;
    glm::dvec2 size() const override;

    void on_mouse_button(const glm::dvec2 &cursor_pos,
                         MouseButton button,
                         Action action,
                         Modifiers) override;
    void on_cursor_move(double x, double y) override;

    sigslot::signal<double> on_drag;

  private:
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
    Transform<double> m_graph_transform;
    bool m_is_dragging = false;
    glm::dvec2 m_cursor_old;
    bool m_is_visible = false;
};
