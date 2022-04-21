#pragma once

#include <memory>
#include <optional>
#include <glm/glm.hpp>
#include "database.hpp"
#include "graph_renderer_opengl.hpp"
#include "window.hpp"
#include "timeseries.hpp"

/**
 * @brief The GraphController stores the ephemeral state of the graph (such as the view matrix and
 * markers) and listens to mouse and key events from the window, modifying the state and redrawing
 * the graph through a renderer accordingly.
 *
 * The GraphController doesn't depend on any specific rendering backend and thus will need to be
 * passed a renderer capable of actually drawing the graph to the screen. The reason for the
 * seperation is so that we could more easily switch out to a new renderer in the future, but mainly
 * so the GraphController can be unit tested without needing to fire up an OpenGL context in the
 * test suite.
 */
class GraphController
{
    struct TimeSeriesContainer
    {
        std::shared_ptr<TimeSeries> ts;
        glm::vec3 colour;
        std::string name;
        bool visible;
        float y_offset;
    };

    struct Marker
    {
        bool visible = false;
        bool is_dragging = false;
        double position;
    };

  public:
    GraphController(Database &database, GraphRendererOpenGL &graph, Window &window);

    /**
     * @brief Get immutable access to the viewmatrix.
     */
    const glm::dvec3 &view_matrix() const;
    void draw();
    void draw_gui();

  private:
    glm::dvec2 screen2graph(const glm::ivec2 &value) const;

    void on_zoom(double x, double y);
    static bool hit_test(glm::ivec2 value, glm::ivec2 tl, glm::ivec2 br);
    void update_view_matrix(const glm::dmat3 &new_view_matrix);

    static constexpr int GUTTER_SIZE_PX = 60;
    static constexpr int TICKLEN_PX = 5;
    static constexpr double ZOOM_MIN_X = 10e6;
    static constexpr double ZOOM_MIN_Y = 10e6;

    Database &m_database;
    GraphRendererOpenGL &m_graph;
    Window &m_window;
    std::vector<TimeSeriesContainer> m_ts;

    // Ephemeral graph state
    int m_plot_width = 2;
    glm::dmat3 m_view_matrix;
    glm::dmat3 m_view_matrix_inv;
    bool m_show_line_segments = false;
    std::pair<Marker, Marker> m_markers;

    // State used for recording screen events
    glm::dvec2 m_cursor_old;
    bool m_is_dragging;
};
