#pragma once

#include <glm/glm.hpp>
#include "graph_renderer_opengl.hpp"
#include "marker_renderer_opengl.hpp"
#include "window.hpp"
#include "graph_state.hpp"

/**
 * @brief The Graph stores the ephemeral state of the graph (such as the view matrix and
 * markers) and listens to mouse and key events from the window, modifying the state and redrawing
 * the graph through a renderer accordingly.
 *
 * The Graph doesn't depend on any specific rendering backend and thus will need to be
 * passed a renderer capable of actually drawing the graph to the screen. The reason for the
 * seperation is so that we could more easily switch out to a new renderer in the future, but mainly
 * so the Graph can be unit tested without needing to fire up an OpenGL context in the
 * test suite.
 */
class Graph
{
  public:
    Graph(GraphRendererOpenGL &renderer, Window &window, GraphState &state);
    glm::dvec2 cursor_gs() const;
    void draw();

  private:
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

    GraphRendererOpenGL &m_renderer;
    Window &m_window;
    GraphState &m_state;

    boost::signals2::scoped_connection m_window_on_scroll_connection;
    boost::signals2::scoped_connection m_window_on_cursor_move_connection;
    boost::signals2::scoped_connection m_window_on_mouse_button_connection;

    struct Marker
    {
        bool is_dragging;
    };

    std::pair<Marker, Marker> m_markers;

    // State used for recording screen events
    glm::dvec2 m_cursor_old;
    bool m_is_dragging = false;
    bool m_is_selecting = false;
    glm::dvec2 m_selection_start;
};
