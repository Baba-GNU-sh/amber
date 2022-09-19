#pragma once

#include <glm/glm.hpp>
#include <sigslot/signal.hpp>
#include <database/timeseries.hpp>
#include "shader_utils.hpp"
#include "window.hpp"
#include "view.hpp"
#include "graph_state.hpp"

class Plot : public View
{
  public:
    Plot(GraphState &state, const Transform<double> &view, Window &m_window);
    ~Plot();
    Plot(const Plot &) = delete;
    Plot &operator=(const Plot &) = delete;
    Plot(Plot &&);
    Plot &operator=(Plot &&) = delete;

    glm::dvec2 position() const override;
    void set_position(const glm::dvec2 &position) override;
    glm::dvec2 size() const override;
    void set_size(const glm::dvec2 &size) override;
    void draw() override;

    sigslot::signal<double> on_zoom;
    sigslot::signal<const glm::dvec2 &> on_pan;

  private:
    void draw_plot(const std::vector<TSSample> &data, glm::vec3 plot_colour, float y_offset) const;
    void on_scroll(const glm::dvec2 &, double, double) override;
    void on_mouse_button(const glm::dvec2 &cursor_pos,
                         MouseButton button,
                         Action action,
                         Modifiers mods) override;
    void on_cursor_move(double, double) override;
    glm::dvec2 screen2graph(const glm::dvec2 &value) const;
    glm::dvec2 screen2graph_delta(const glm::dvec2 &value) const;
    glm::dvec2 graph2screen(const glm::dvec2 &value) const;

    GraphState &m_state;
    const Transform<double> &m_view;
    Window &m_window;
    static constexpr size_t PIXELS_PER_COL = 1;
    static constexpr size_t COLS_MAX = 8192; // Number of preallocated buffer space for samples
    unsigned int m_vao;
    unsigned int m_vbo;
    Program m_shader;
    glm::dvec2 m_position;
    glm::dvec2 m_size;

    bool m_is_dragging = false;
    glm::dvec2 m_cursor_pos_old;
};
