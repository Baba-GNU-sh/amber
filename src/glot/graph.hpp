#pragma once

#include <glm/glm.hpp>
#include "marker.hpp"
#include "plot.hpp"
#include "selection_box.hpp"
#include "window.hpp"
#include "graph_state.hpp"

class Graph
{
  public:
    Graph(Window &window, GraphState &state);
    ~Graph();
    Graph(const Graph &) = delete;
    Graph(Graph &&) = delete;
    Graph &operator=(const Graph &) = delete;
    Graph &operator=(Graph &&) = delete;

    glm::dvec2 cursor_gs() const;
    void draw();

  private:
    // void on_scroll(double, double) override;
    // void on_zoom(double, double) override;
    // void on_mouse_button(int, int, int) override;
    // void on_cursor_move(double, double) override;
    // void on_key(int, int, int, int) override;

    void handle_scroll(double xoffset, double yoffset);
    void handle_cursor_move(double xpos, double ypos);
    void handle_mouse_button(int button, int action, int mods);
    void init_line_buffers();
    void draw_lines();
    void draw_labels();
    void draw_plots();
    void draw_markers();
    std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> tick_spacing() const;
    glm::dvec2 screen2graph(const glm::ivec2 &value) const;
    glm::dvec2 screen2graph_delta(const glm::ivec2 &value) const;
    glm::dvec2 graph2screen(const glm::dvec2 &value) const;
    void on_zoom(double x, double y);

    static constexpr int GUTTER_SIZE_PX = 60;
    static constexpr int TICKLEN_PX = 5;
    static constexpr double ZOOM_MAX = 10e6;
    static constexpr int PIXELS_PER_COL = 1;

    Window &m_window;
    GraphState &m_state;
    Font m_font;
    std::vector<Label> m_axis_labels;
    std::vector<Label> m_marker_ts_labels;
    std::vector<Plot> m_plots;
    Marker m_marker_a;
    Marker m_marker_b;
    SelectionBox m_selection_box;
    glm::ivec2 m_size;

    // Line buffers - TODO move these to some other primitive class thing
    GLuint m_linebuf_vao;
    GLuint m_linebuf_vbo;
    Program m_lines_shader;

    glm::dvec2 m_cursor_old;
    bool m_is_dragging = false;
    bool m_is_selecting = false;
    glm::dvec2 m_selection_start;
};
