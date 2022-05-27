#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <utility>
#include <imgui.h>
#include <sstream>
#include "graph.hpp"
#include "plot.hpp"
#include "marker.hpp"
#include "resources.hpp"
#include "graph_utils.hpp"

Graph::Graph(GraphState &state)
    : m_state(state), m_axis_horizontal(Axis::Orientation::Horizontal, m_state.view),
      m_axis_vertical(Axis::Orientation::Vertical, m_state.view)
{
    using namespace std::placeholders;

    state.view.set_zoom_limit(ZOOM_MAX);

    m_axis_horizontal.on_zoom.connect([this](const Window &window, double amount) {
        const auto delta = 1.0 + amount * 0.1;
        apply_zoom(window, glm::dvec2(delta, 1.0));
    });

    m_axis_vertical.on_zoom.connect([this](const Window &window, double amount) {
        const auto delta = 1.0 + amount * 0.1;
        apply_zoom(window, glm::dvec2(1.0, delta));
    });

    m_plot.on_zoom.connect([this](const Window &window, double amount) {
        const auto delta = 1.0 + amount * 0.1;
        apply_zoom(window, glm::dvec2(delta));
    });

    // for (int i = 0; i < 128; ++i)
    // {
    //     m_axis_labels.emplace_back(m_font);
    //     m_axis_labels.back().set_colour(glm::vec3(1.0, 1.0, 1.0));
    // }

    // for (auto &ts : m_state.timeseries)
    // {
    //     (void)ts;
    //     m_marker_ts_labels.emplace_back(m_font);
    //     m_marker_ts_labels.emplace_back(m_font);
    //     m_plots.emplace_back(m_window);
    // }

    // m_size = m_window.size();
    // m_marker_a.set_colour(glm::vec3(0.0, 1.0, 1.0));
    // m_marker_b.set_colour(glm::vec3(1.0, 1.0, 0.0));
}

Graph::~Graph()
{
    // glDeleteBuffers(1, &m_linebuf_vbo);
    // glDeleteVertexArrays(1, &m_linebuf_vbo);
}

glm::dvec2 Graph::cursor_gs() const
{
    // return screen2graph(m_cursor_old);
    return glm::dvec2(0.0);
}

void Graph::draw(const Window &window) const
{
    if (m_state.sync_latest_data)
    {
        m_state.goto_newest_sample();
    }

    m_axis_horizontal.draw(window);
    m_axis_vertical.draw(window);

    // draw_lines();
    // draw_labels();
    // draw_plots();
    // draw_markers();

    // if (m_is_selecting)
    // {
    //     m_selection_box.set_position(m_selection_start);
    //     m_selection_box.set_size(m_window.cursor() - m_selection_start);
    //     m_selection_box.draw();
    // }
}

void Graph::on_resize(int width, int height)
{
    m_position = glm::dvec2(0.0);
    m_size = glm::dvec2(width, height);
    layout();
}

void Graph::on_scroll(const Window &window, double x, double y)
{
    (void)x;

    if (hit_test(window, m_axis_horizontal))
        m_axis_horizontal.on_scroll(window, x, y);

    if (hit_test(window, m_axis_vertical))
        m_axis_vertical.on_scroll(window, x, y);

    if (hit_test(window, m_plot))
        m_plot.on_scroll(window, x, y);
}

glm::dvec2 Graph::size() const
{
    return m_size;
}

glm::dvec2 Graph::position() const
{
    return m_position;
}

void Graph::layout()
{
    m_axis_horizontal.set_position(glm::dvec2(GUTTER_SIZE, m_size.y - GUTTER_SIZE) + m_position);
    m_axis_horizontal.set_size(glm::dvec2(m_size.x - GUTTER_SIZE, GUTTER_SIZE));

    m_axis_vertical.set_position(glm::dvec2(0.0) + m_position);
    m_axis_vertical.set_size(glm::dvec2(GUTTER_SIZE, m_size.y - GUTTER_SIZE));

    m_plot.set_position(glm::dvec2(m_position.x + GUTTER_SIZE, m_position.y));
    m_plot.set_size(m_size - glm::dvec2(GUTTER_SIZE));
}

bool Graph::hit_test(const Window &window, const View &view)
{
    return GraphUtils::hit_test(window.cursor(), view.position(), view.position() + view.size());
}

glm::dvec2 Graph::screen2graph(const Transform<double> &viewport_txform,
                               const glm::ivec2 &viewport_space) const
{
    const auto clip_space = viewport_txform.apply_inverse(viewport_space);
    const auto graph_space = m_state.view.apply_inverse(clip_space);
    return graph_space;
}

glm::dvec2 Graph::screen2graph_delta(const Transform<double> &viewport_txform,
                                     const glm::ivec2 &delta) const
{
    auto begin_gs = screen2graph(viewport_txform, glm::ivec2(0, 0));
    auto end_gs = screen2graph(viewport_txform, glm::ivec2(0, 0) + delta);
    return end_gs - begin_gs;
}

glm::dvec2 Graph::graph2screen(const Transform<double> &viewport_txform,
                               const glm::dvec2 &value) const
{
    const auto clip_space = m_state.view.apply(value);
    const auto screen_space = viewport_txform.apply(clip_space);
    return screen_space;
}

// void Graph::handle_scroll(double /*xoffset*/, double yoffset)
// {
//     const double zoom_delta = 1.0f + (yoffset / 10.0f);
//     const auto cursor = m_window.cursor();
//     const auto size = m_window.size();

//     if (GraphUtils::hit_test(
//             cursor, glm::ivec2(0, 0), glm::ivec2(GUTTER_SIZE_PX, size.y - GUTTER_SIZE_PX)))
//     {
//         // Cursor is in the vertical gutter, only zoom the y axis
//         on_zoom(1.0, zoom_delta);
//     }
//     else if (GraphUtils::hit_test(cursor,
//                                   glm::ivec2(GUTTER_SIZE_PX, size.y - GUTTER_SIZE_PX),
//                                   glm::ivec2(size.x, size.y)))
//     {
//         // Cursor is in the horizontal gutter, only zoom the x axis
//         on_zoom(zoom_delta, 1.0);
//     }
//     else if (GraphUtils::hit_test(cursor,
//                                   glm::ivec2(GUTTER_SIZE_PX, 0),
//                                   glm::ivec2(size.x, size.y - GUTTER_SIZE_PX)))
//     {
//         // Cursor is in the main part of the graph, zoom both axes
//         on_zoom(zoom_delta, zoom_delta);
//     }
// }

// void Graph::handle_cursor_move(double xpos, double ypos)
// {
//     glm::dvec2 cursor(xpos, ypos);

//     // Work out how much the cursor moved since the last time
//     const auto cursor_delta = cursor - m_cursor_old;

//     // Work out the delta in graph space
//     const auto txform =
//         m_state.view.matrix_inverse() * m_window.viewport_transform().matrix_inverse();
//     const auto a = txform * glm::dvec3(0.0f);
//     const auto b = txform * glm::dvec3(cursor_delta, 0.0f);
//     const auto delta = b - a;

//     // Convert the delta back to a 2D vector
//     glm::dvec2 cursor_gs_delta(delta.x, delta.y);

//     if (m_is_dragging)
//     {
//         m_state.view.translate(cursor_gs_delta);
//         // m_state.update_view_matrix(glm::translate(m_state.view.get(), cursor_gs_delta));
//     }

//     if (m_marker_a.is_dragging)
//     {
//         m_state.markers.first.position = m_state.markers.first.position + cursor_gs_delta.x;
//     }

//     if (m_marker_b.is_dragging)
//     {
//         m_state.markers.second.position = m_state.markers.second.position + cursor_gs_delta.x;
//     }

//     // Cache the position of the cursor for next time
//     m_cursor_old = cursor;
// }

// void Graph::handle_mouse_button(int button, int action, int /*mods*/)
// {
//     if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
//     {
//         // Work out what the user has clicked on
//         auto marker_clicked = [this](GraphState::MarkerState &marker) -> bool {
//             if (!marker.visible)
//                 return false;

//             const auto marker_pos_vs = graph2screen(glm::dvec2(marker.position, 0.0));
//             const auto cursor = m_window.cursor();

//             return (std::abs(cursor.x - marker_pos_vs.x) < 8);
//         };
//         if (marker_clicked(m_state.markers.first))
//         {
//             m_marker_a.is_dragging = true;
//         }
//         else if (marker_clicked(m_state.markers.second))
//         {
//             m_marker_b.is_dragging = true;
//         }
//         else
//         {
//             m_is_dragging = true;
//         }
//     }
//     else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
//     {
//         m_is_dragging = false;
//         m_marker_a.is_dragging = false;
//         m_marker_b.is_dragging = false;
//     }

//     if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
//     {
//         m_is_selecting = true;
//         m_selection_start = m_window.cursor();
//     }
//     else if (button == GLFW_MOUSE_BUTTON_RIGHT)
//     {
//         m_is_selecting = false;

//         // Move the view_matrix to fit the selection box
//         const auto start_gs = screen2graph(m_selection_start);
//         const auto end_gs = screen2graph(m_window.cursor());

//         m_state.fit_graph(start_gs, end_gs);
//     }
// }

// glm::dvec2 Graph::screen2graph(const glm::ivec2 &viewport_space) const
// {
//     const auto &graphview_txform = m_state.view;
//     const auto &viewport_txform = m_window.viewport_transform();

//     const auto clip_space = viewport_txform.apply_inverse(viewport_space);
//     const auto graph_space = graphview_txform.apply_inverse(clip_space);

//     return graph_space;
// }

// glm::dvec2 Graph::screen2graph_delta(const glm::ivec2 &delta) const
// {
//     auto begin_gs = screen2graph(glm::ivec2(0, 0));
//     auto end_gs = screen2graph(glm::ivec2(0, 0) + delta);
//     return end_gs - begin_gs;
// }

// glm::dvec2 Graph::graph2screen(const glm::dvec2 &value) const
// {
//     return m_window.viewport_transform().apply(m_state.view.apply(value));
// }

void Graph::apply_zoom(const Window &window, const glm::dvec2 &zoom_delta_vec)
{
    // Store where the pointer is in graph space before scaling
    const auto cursor_in_gs_old = screen2graph(window.viewport_transform(), window.cursor());

    // Scale the view matrix by the zoom amount, clamping to some max value
    m_state.view.scale(zoom_delta_vec);

    // Work out where the cursor would be under this new zoom level and recenter the view on the
    // cursor
    const auto cursor_in_gs_new = screen2graph(window.viewport_transform(), window.cursor());
    auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;
    m_state.view.translate(cursor_delta);
}

// void Graph::on_zoom(Window &window, const glm::dvec2 &zoom_delta_vec)
// {
//     // Store where the pointer is in graph space before scaling
//     const auto cursor_in_gs_old = screen2graph(window.cursor());

//     // Scale the view matrix by the zoom amount
//     m_state.view.scale(zoom_delta_vec);

//     // Work out where the cursor would be under this new zoom level and recenter the view on the
//     // cursor
//     const auto cursor_in_gs_new = screen2graph(window.cursor());
//     auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;
//     m_state.view.translate(cursor_delta);
// }
