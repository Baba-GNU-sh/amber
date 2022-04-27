#include "graph.hpp"
#include "plot_renderer_opengl.hpp"
#include <imgui.h>
#include <utility>

Graph::Graph(GraphRendererOpenGL &renderer, Window &window, GraphState &state)
    : m_renderer(renderer), m_window(window), m_state(state)
{
    using namespace std::placeholders;

    m_window_on_scroll_connection =
        m_window.on_scroll(std::bind(&Graph::handle_scroll, this, _1, _2));

    m_window_on_cursor_move_connection =
        m_window.on_cursor_move(std::bind(&Graph::handle_cursor_move, this, _1, _2));

    m_window_on_mouse_button_connection =
        m_window.on_mouse_button(std::bind(&Graph::handle_mouse_button, this, _1, _2, _3));
}

glm::dvec2 Graph::cursor_gs() const
{
    return screen2graph(m_cursor_old);
}

void Graph::draw()
{
    m_renderer.set_gutter_size(GUTTER_SIZE_PX);
    m_renderer.set_tick_len(TICKLEN_PX);
    m_renderer.set_view_matrix(m_state.view_matrix);
    m_renderer.set_size(m_window.size());

    m_renderer.draw_graph();

    const auto graph_size_px = m_window.size();
    const auto plot_size_px = graph_size_px - glm::ivec2(GUTTER_SIZE_PX, GUTTER_SIZE_PX);
    const auto plot_position_px = glm::ivec2(GUTTER_SIZE_PX, 0);

    const int num_samples = plot_size_px.x / PIXELS_PER_COL;

    const auto plot_position_gs = screen2graph(plot_position_px);
    const auto plot_size_gs = screen2graph_delta(plot_size_px);
    const auto interval_gs = PIXELS_PER_COL * plot_size_gs.x / num_samples;

    for (auto &time_series : m_state.timeseries)
    {
        if (time_series.visible)
        {
            std::vector<TSSample> samples(num_samples);
            auto n_samples = time_series.ts->get_samples(
                samples.data(), plot_position_gs.x, interval_gs, num_samples);
            samples.resize(n_samples);

            m_renderer.draw_plot(samples,
                                 m_state.plot_width,
                                 time_series.colour,
                                 time_series.y_offset,
                                 m_state.show_line_segments);
        }
    }

    auto marker_styles = [this] {
        const auto both_markers_visible =
            m_state.markers.first.visible && m_state.markers.second.visible;
        if (!both_markers_visible)
        {
            return std::make_pair(MarkerRendererOpenGL::MarkerStyle::Center,
                                  MarkerRendererOpenGL::MarkerStyle::Center);
        }

        if (m_state.markers.first.position < m_state.markers.second.position)
        {
            return std::make_pair(MarkerRendererOpenGL::MarkerStyle::Left,
                                  MarkerRendererOpenGL::MarkerStyle::Right);
        }
        else
        {
            return std::make_pair(MarkerRendererOpenGL::MarkerStyle::Right,
                                  MarkerRendererOpenGL::MarkerStyle::Left);
        }
    }();

    if (m_state.markers.first.visible)
    {
        m_renderer.draw_marker(
            m_state.markers.first.position, marker_styles.first, glm::vec3(0.0, 1.0, 1.0));

        for (const auto &time_series : m_state.timeseries)
        {
            const auto value =
                time_series.ts->get_sample(m_state.markers.first.position, interval_gs);
            m_renderer.draw_value_label(
                glm::dvec2(m_state.markers.first.position, value.average + time_series.y_offset),
                value.average,
                time_series.colour);
        }
    }

    if (m_state.markers.second.visible)
    {
        m_renderer.draw_marker(
            m_state.markers.second.position, marker_styles.second, glm::vec3(1.0, 1.0, 0.0));
        for (const auto &time_series : m_state.timeseries)
        {
            const auto value =
                time_series.ts->get_sample(m_state.markers.second.position, interval_gs);
            m_renderer.draw_value_label(
                glm::dvec2(m_state.markers.second.position, value.average + time_series.y_offset),
                value.average,
                time_series.colour);
        }
    }

    if (m_is_selecting)
    {
        // Draw a box of solid colour with alpha == .5 between m_selection_start and cursor
        m_renderer.draw_selection_box(m_selection_start, m_window.cursor());
    }
}
void Graph::handle_scroll(double /*xoffset*/, double yoffset)
{
    const double zoom_delta = 1.0f + (yoffset / 10.0f);
    const auto cursor = m_window.cursor();
    const auto size = m_window.size();

    if (hit_test(cursor, glm::ivec2(0, 0), glm::ivec2(GUTTER_SIZE_PX, size.y - GUTTER_SIZE_PX)))
    {
        // Cursor is in the vertical gutter, only zoom the y axis
        on_zoom(1.0, zoom_delta);
    }
    else if (hit_test(cursor,
                      glm::ivec2(GUTTER_SIZE_PX, size.y - GUTTER_SIZE_PX),
                      glm::ivec2(size.x, size.y)))
    {
        // Cursor is in the horizontal gutter, only zoom the x axis
        on_zoom(zoom_delta, 1.0);
    }
    else if (hit_test(cursor,
                      glm::ivec2(GUTTER_SIZE_PX, 0),
                      glm::ivec2(size.x, size.y - GUTTER_SIZE_PX)))
    {
        // Cursor is in the main part of the graph, zoom both axes
        on_zoom(zoom_delta, zoom_delta);
    }
}

void Graph::handle_cursor_move(double xpos, double ypos)
{
    glm::dvec2 cursor(xpos, ypos);

    // Work out how much the cursor moved since the last time
    const auto cursor_delta = cursor - m_cursor_old;

    // Work out the delta in graph space
    const auto txform = m_state.view_matrix_inv * glm::dmat3(m_window.vp_matrix_inv());
    const auto a = txform * glm::dvec3(0.0f);
    const auto b = txform * glm::dvec3(cursor_delta, 0.0f);
    const auto delta = b - a;

    // Convert the delta back to a 2D vector
    glm::dvec2 cursor_gs_delta(delta.x, delta.y);

    if (m_is_dragging)
    {
        m_state.update_view_matrix(glm::translate(m_state.view_matrix, cursor_gs_delta));
    }

    if (m_markers.first.is_dragging)
    {
        m_state.markers.first.position = m_state.markers.first.position + cursor_gs_delta.x;
    }

    if (m_markers.second.is_dragging)
    {
        m_state.markers.second.position = m_state.markers.second.position + cursor_gs_delta.x;
    }

    // Cache the position of the cursor for next time
    m_cursor_old = cursor;
}

void Graph::handle_mouse_button(int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Work out what the user has clicked on
        auto marker_clicked = [this](GraphState::MarkerState &marker) -> bool {
            if (!marker.visible)
                return false;

            glm::vec3 marker_pos_vs = m_window.vp_matrix() *
                                      (m_state.view_matrix * glm::dvec3(marker.position, 0.0, 1.0));
            auto cursor = m_window.cursor();

            return (std::abs(cursor.x - marker_pos_vs.x) < 8);
        };
        if (marker_clicked(m_state.markers.first))
        {
            m_markers.first.is_dragging = true;
        }
        else if (marker_clicked(m_state.markers.second))
        {
            m_markers.second.is_dragging = true;
        }
        else
        {
            m_is_dragging = true;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        m_is_dragging = false;
        m_markers.first.is_dragging = false;
        m_markers.second.is_dragging = false;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        m_is_selecting = true;
        m_selection_start = m_window.cursor();
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        m_is_selecting = false;

        // Move the view_matrix to fit the selection box
        const auto start_gs = screen2graph(m_selection_start);
        const auto end_gs = screen2graph(m_window.cursor());

        fit_graph(start_gs, end_gs);
    }
}

glm::dvec2 Graph::screen2graph(const glm::ivec2 &value) const
{
    const glm::dvec3 value3(value, 1.0f);
    glm::dvec3 value_cs = m_window.vp_matrix_inv() * value3;
    glm::dvec3 value_gs = glm::inverse(m_state.view_matrix) * value_cs;
    return value_gs;
}

glm::dvec2 Graph::screen2graph_delta(const glm::ivec2 &delta) const
{
    auto begin_gs = screen2graph(glm::ivec2(0, 0));
    auto end_gs = screen2graph(glm::ivec2(0, 0) + delta);
    return end_gs - begin_gs;
}

void Graph::on_zoom(double x, double y)
{
    // Make a vector from the zoom delta
    glm::dvec2 zoom_delta_vec(x, y);

    // Store where the pointer is in graph space before scaling
    const auto cursor_in_gs_old = screen2graph(m_window.cursor());

    // Scale the view matrix by the zoom amount, clamping to some max value
    auto view_matrix_zoomed = glm::scale(m_state.view_matrix, zoom_delta_vec);
    view_matrix_zoomed[0][0] = std::min(view_matrix_zoomed[0][0], ZOOM_MIN_X);
    view_matrix_zoomed[1][1] = std::min(view_matrix_zoomed[1][1], ZOOM_MIN_Y);
    m_state.update_view_matrix(view_matrix_zoomed);

    // Work out where the cursor would be under this new zoom level and recenter the view on the
    // cursor
    const auto cursor_in_gs_new = screen2graph(m_window.cursor());
    auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;
    m_state.update_view_matrix(glm::translate(m_state.view_matrix, cursor_delta));
}

bool Graph::hit_test(glm::ivec2 value, glm::ivec2 tl, glm::ivec2 br)
{
    if (value.x < tl.x)
        return false;
    if (value.x >= br.x)
        return false;
    if (value.y < tl.y)
        return false;
    if (value.y >= br.y)
        return false;
    return true;
}

void Graph::fit_graph(const glm::dvec2 &start, const glm::dvec2 &end)
{
    glm::dmat3 view_matrix(1.0);

    const auto delta = glm::abs(end - start);
    const auto scaling_factor = 2.0 / delta;
    view_matrix = glm::scale(view_matrix, scaling_factor);
    
    const auto translation = (start + end) / 2.0;
    view_matrix = glm::translate(view_matrix, -translation);

    m_state.update_view_matrix(view_matrix);
}
