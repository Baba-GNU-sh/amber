#pragma once

#include <glm/glm.hpp>
#include "line_renderer_opengl.hpp"
#include "marker.hpp"
#include "plot.hpp"
#include "selection_box.hpp"
#include "window.hpp"
#include "graph_state.hpp"

class Graph
{
  public:
    Graph(Window &window, GraphState &state);
    glm::dvec2 cursor_gs() const;
    void draw();

  private:
    void init_line_buffers();
    void draw_lines();
    void draw_labels();
    void draw_plots();
    void draw_markers();
    std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> tick_spacing() const;
    void handle_scroll(double xoffset, double yoffset);
    void handle_cursor_move(double xpos, double ypos);
    void handle_mouse_button(int button, int action, int mods);
    glm::dvec2 screen2graph(const glm::ivec2 &value) const;
    glm::dvec2 screen2graph_delta(const glm::ivec2 &value) const;
    void on_zoom(double x, double y);
    static bool hit_test(glm::ivec2 value, glm::ivec2 tl, glm::ivec2 br);
    void fit_graph(const glm::dvec2 &start, const glm::dvec2 &end);

    static constexpr int GUTTER_SIZE_PX = 60;
    static constexpr int TICKLEN_PX = 5;
    static constexpr double ZOOM_MIN_X = 10e6;
    static constexpr double ZOOM_MIN_Y = 10e6;
    static constexpr int PIXELS_PER_COL = 1;

    Window &m_window;
    GraphState &m_state;
    FontMaterial m_font;
    std::vector<Label> m_axis_labels;
    std::vector<Label> m_marker_ts_labels;
    std::vector<Plot> m_plots;
    Marker m_marker_a;
    Marker m_marker_b;
    SelectionBox m_selection_box;

    // Line buffers - TODO move these to some other primitive class thing
    GLuint _linebuf_vao;
    GLuint _linebuf_vbo;
    Program _lines_shader;

    glm::ivec2 m_size;

    boost::signals2::scoped_connection m_window_on_scroll_connection;
    boost::signals2::scoped_connection m_window_on_cursor_move_connection;
    boost::signals2::scoped_connection m_window_on_mouse_button_connection;

    glm::dvec2 m_cursor_old;
    bool m_is_dragging = false;
    bool m_is_selecting = false;
    glm::dvec2 m_selection_start;
};
