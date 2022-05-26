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
    Graph(GraphState &state);
    ~Graph();
    Graph(const Graph &) = delete;
    Graph(Graph &&) = delete;
    Graph &operator=(const Graph &) = delete;
    Graph &operator=(Graph &&) = delete;

    glm::dvec2 cursor_gs() const;
    void draw(const Window &window) const override;
    void on_resize(int, int) override;
    void on_scroll(const Window &, double, double) override;
    glm::dvec2 size() const override;
    glm::dvec2 position() const override;

  private:
    void layout();
    // glm::dvec2 screen2graph(const glm::ivec2 &value) const;
    // glm::dvec2 screen2graph_delta(const glm::ivec2 &value) const;
    // glm::dvec2 graph2screen(const glm::dvec2 &value) const;

    static bool hit_test(const Window &window, const View &view);

    static constexpr double GUTTER_SIZE = 60;
    static constexpr double ZOOM_MAX = 10e6;
    static constexpr int PIXELS_PER_COL = 1;

    GraphState &m_state;
    Axis m_axis_horizontal;
    Axis m_axis_vertical;
    // Plot m_plot;

    // std::vector<Label> m_axis_labels;
    // std::vector<Label> m_marker_ts_labels;
    // std::vector<Plot> m_plots;
    // Marker m_marker_a;
    // Marker m_marker_b;
    // SelectionBox m_selection_box;
    glm::dvec2 m_size;
    glm::dvec2 m_position;

    // Line buffers - TODO move these to some other primitive class thing
    // GLuint m_linebuf_vao;
    // GLuint m_linebuf_vbo;
    // Program m_lines_shader;

    // glm::dvec2 m_cursor_old;
    // bool m_is_dragging = false;
    // bool m_is_selecting = false;
    // glm::dvec2 m_selection_start;
};
