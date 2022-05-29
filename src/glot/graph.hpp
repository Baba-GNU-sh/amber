#pragma once

#include <glm/glm.hpp>
#include "marker.hpp"
#include "plot.hpp"
#include "selection_box.hpp"
#include "window.hpp"
#include "graph_state.hpp"
#include "view.hpp"
#include "axis.hpp"

class Graph : public View
{
  public:
    Graph(GraphState &state, Window &window);
    ~Graph() = default;
    Graph(const Graph &) = delete;
    Graph(Graph &&) = delete;
    Graph &operator=(const Graph &) = delete;
    Graph &operator=(Graph &&) = delete;

    glm::dvec2 cursor_gs() const;
    void draw(const Window &window) const override;
    void on_resize(int, int) override;

    glm::dvec2 size() const override;
    glm::dvec2 position() const override;
    void on_mouse_button(const glm::dvec2 &cursor_pos, int button, int action, int mods) override;
    void on_cursor_move(Window &window, double xpos, double ypos) override;

  private:
    void layout();

    glm::dvec2 screen2graph(const glm::ivec2 &value) const;
    glm::dvec2 screen2graph_delta(const glm::ivec2 &value) const;
    glm::dvec2 graph2screen(const Transform<double> &viewport_txform,
                            const glm::dvec2 &value) const;

    void apply_zoom(const Window &window, const glm::dvec2 &);

    static constexpr double GUTTER_SIZE = 60;
    static constexpr double ZOOM_MAX = 10e6;
    static constexpr int PIXELS_PER_COL = 1;

    GraphState &m_state;
    Window &m_window;
    Axis<AxisHorizontal> m_axis_horizontal;
    Axis<AxisVertical> m_axis_vertical;
    Plot m_plot;
    Marker m_marker_a;
    Marker m_marker_b;
    SelectionBox m_selection_box;

    glm::dvec2 m_size;
    glm::dvec2 m_position;

    // glm::dvec2 m_cursor_old;
    // bool m_is_dragging = false;
    bool m_is_selecting = false;
    glm::dvec2 m_selection_start;
};
