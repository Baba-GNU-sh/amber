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

Graph::Graph(GraphState &state, Window &window)
    : m_state(state), m_window(window), m_axis_horizontal(window), m_axis_vertical(window),
      m_plot(state, window), m_marker_a(window), m_marker_b(window), m_selection_box(window)
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

    m_plot.on_pan.connect([this](const Window &, glm::dvec2 amount) {
        const auto delta_gs = screen2graph_delta(amount);
        m_state.view.translate(delta_gs);
        m_axis_horizontal.set_graph_transform(m_state.view);
        m_axis_vertical.set_graph_transform(m_state.view);
        m_marker_a.set_graph_transform(m_state.view);
        m_marker_b.set_graph_transform(m_state.view);
    });

    m_marker_a.on_drag.connect([this](double delta) {
        const auto position_gs = screen2graph_delta(glm::dvec2(delta, 0));
        m_marker_a.set_x_position(m_marker_a.x_position() + position_gs.x);
    });

    m_marker_b.on_drag.connect([this](double delta) {
        const auto position_gs = screen2graph_delta(glm::dvec2(delta, 0));
        m_marker_b.set_x_position(m_marker_b.x_position() + position_gs.x);
    });

    add_view(&m_axis_horizontal);
    add_view(&m_axis_vertical);
    add_view(&m_plot);
    add_view(&m_marker_a);
    add_view(&m_marker_b);
    add_view(&m_selection_box);

    m_marker_a.set_x_position(0.0);
    m_marker_b.set_x_position(0.0);

    m_marker_a.set_colour(glm::vec3(0.0, 1.0, 1.0));
    m_marker_b.set_colour(glm::vec3(1.0, 1.0, 0.0));
}

glm::dvec2 Graph::cursor_gs() const
{
    return screen2graph(m_window.cursor());
}

void Graph::draw(const Window &window) const
{
    View::draw(window);
}

void Graph::on_resize(int width, int height)
{
    m_position = glm::dvec2(0.0);
    m_size = glm::dvec2(width, height);
    layout();
}

glm::dvec2 Graph::size() const
{
    return m_size;
}

glm::dvec2 Graph::position() const
{
    return m_position;
}

void Graph::on_mouse_button(const glm::dvec2 &cursor_pos, int button, int action, int mods)
{
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        m_is_selecting = true;
        m_selection_start = m_window.cursor();
        m_selection_box.set_visible(true);
        m_selection_box.set_position(m_selection_start);
        m_selection_box.set_size(glm::dvec2(0.0));
    }
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        m_is_selecting = false;
        m_state.fit_graph(screen2graph(m_selection_start), screen2graph(m_window.cursor()));
        m_selection_box.set_visible(false);
        m_axis_horizontal.set_graph_transform(m_state.view);
        m_axis_vertical.set_graph_transform(m_state.view);
    }
    else
    {
        View::on_mouse_button(cursor_pos, button, action, mods);
    }
}

void Graph::on_cursor_move(Window &window, double x, double y)
{
    if (m_is_selecting)
    {
        m_selection_box.set_position(m_selection_start);
        m_selection_box.set_size(m_window.cursor() - m_selection_start);
    }

    View::on_cursor_move(window, x, y);
}

bool Graph::marker_is_visible(MarkerType m) const
{
    const auto &marker = get_marker(m);
    return marker.is_visible();
}
double Graph::marker_position(MarkerType m) const
{
    const auto &marker = get_marker(m);
    return marker.x_position();
}
void Graph::set_marker_visible(MarkerType m, bool visible)
{
    auto &marker = get_marker(m);
    marker.set_visible(visible);
}
void Graph::set_marker_position(MarkerType m, double position)
{
    auto &marker = get_marker(m);
    marker.set_x_position(position);
}

void Graph::layout()
{
    m_axis_horizontal.set_position(glm::dvec2(GUTTER_SIZE, m_size.y - GUTTER_SIZE) + m_position);
    m_axis_horizontal.set_size(glm::dvec2(m_size.x - GUTTER_SIZE, GUTTER_SIZE));

    m_axis_vertical.set_position(glm::dvec2(0.0) + m_position);
    m_axis_vertical.set_size(glm::dvec2(GUTTER_SIZE, m_size.y - GUTTER_SIZE));

    m_plot.set_position(glm::dvec2(m_position.x + GUTTER_SIZE, m_position.y));
    m_plot.set_size(m_size - glm::dvec2(GUTTER_SIZE));

    m_marker_a.set_screen_height(m_size.y - GUTTER_SIZE);
    m_marker_b.set_screen_height(m_size.y - GUTTER_SIZE);
}

void Graph::reveal_newest_sample()
{
    // Zero out the x translation element of the view transform - centers the view on time=0
    auto view_matrix_copy = m_state.view.matrix();
    view_matrix_copy[2][0] = 0;
    m_state.view.update(view_matrix_copy);

    // Center the view on the latest sample
    m_state.view.translate(glm::dvec2(-latest_visibile_sample_time(), 0));

    // Translate the view to the left by half a screen, to align the latest sample with the
    // right edge of the view
    const auto center_offset = m_state.view.apply_inverse_relative(glm::dvec2(1.0, 0.0));
    m_state.view.translate(center_offset);
}

/**
 * @brief Evalulate the time latest (newest) visible sample.
 */
double Graph::latest_visibile_sample_time() const
{
    double latest_sample_time = 0.0;
    for (std::size_t i = 0; i < m_state.timeseries.size(); ++i)
    {
        const auto &ts = m_state.timeseries[i];
        if (ts.visible)
        {
            const auto span = ts.ts->get_span();
            latest_sample_time = std::max(latest_sample_time, span.second);
        }
    }
    return latest_sample_time;
}

glm::dvec2 Graph::screen2graph(const glm::dvec2 &viewport_space) const
{
    const auto clip_space = m_window.viewport_transform().apply_inverse(viewport_space);
    const auto graph_space = m_state.view.apply_inverse(clip_space);
    return graph_space;
}

glm::dvec2 Graph::screen2graph_delta(const glm::dvec2 &delta) const
{
    auto begin_gs = screen2graph(glm::dvec2(0, 0));
    auto end_gs = screen2graph(glm::dvec2(0, 0) + delta);
    return end_gs - begin_gs;
}

glm::dvec2 Graph::graph2screen(const glm::dvec2 &value) const
{
    const auto clip_space = m_state.view.apply(value);
    const auto screen_space = m_window.viewport_transform().apply(clip_space);
    return screen_space;
}

void Graph::apply_zoom(const Window &window, const glm::dvec2 &zoom_delta_vec)
{
    // Store where the pointer is in graph space before scaling
    const auto cursor_in_gs_old = screen2graph(window.cursor());

    // Scale the view matrix by the zoom amount, clamping to some max value
    m_state.view.scale(zoom_delta_vec);

    // Work out where the cursor would be under this new zoom level and recenter the view on the
    // cursor
    const auto cursor_in_gs_new = screen2graph(window.cursor());
    auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;
    m_state.view.translate(cursor_delta);

    m_axis_horizontal.set_graph_transform(m_state.view);
    m_axis_vertical.set_graph_transform(m_state.view);
    m_marker_a.set_graph_transform(m_state.view);
    m_marker_b.set_graph_transform(m_state.view);
}
