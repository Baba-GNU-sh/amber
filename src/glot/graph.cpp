#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <utility>
#include <imgui.h>
#include "graph.hpp"
#include "plot_renderer_opengl.hpp"
#include "marker.hpp"
#include "resources.hpp"

Graph::Graph(Window &window, GraphState &state)
    : m_window(window), m_state(state), m_font("proggy_clean.png"), m_marker_a(m_window),
      m_marker_b(m_window), m_line_renderer(m_window)
{
    using namespace std::placeholders;

    m_window.on_resize([this](int, int) { m_size = m_window.size(); });

    m_window_on_scroll_connection =
        m_window.on_scroll(std::bind(&Graph::handle_scroll, this, _1, _2));

    m_window_on_cursor_move_connection =
        m_window.on_cursor_move(std::bind(&Graph::handle_cursor_move, this, _1, _2));

    m_window_on_mouse_button_connection =
        m_window.on_mouse_button(std::bind(&Graph::handle_mouse_button, this, _1, _2, _3));

    init_line_buffers();

    for (int i = 0; i < 128; ++i)
    {
        m_axis_labels.emplace_back(m_window, m_font);
        m_axis_labels.back().set_colour(glm::vec3(1.0, 1.0, 1.0));
    }

    for (auto &ts : m_state.timeseries)
    {
        (void)ts;
        m_marker_ts_labels.emplace_back(m_window, m_font);
        m_marker_ts_labels.emplace_back(m_window, m_font);
        m_plots.emplace_back(m_window);
    }

    m_size = m_window.size();
    m_marker_a.set_colour(glm::vec3(0.0, 1.0, 1.0));
    m_marker_b.set_colour(glm::vec3(1.0, 1.0, 0.0));
}

glm::dvec2 Graph::cursor_gs() const
{
    return screen2graph(m_cursor_old);
}

void Graph::draw()
{
    if (m_state.sync_latest_data)
    {
        m_state.goto_newest_sample();
    }

    draw_lines();
    draw_labels();
    draw_plots();
    draw_markers();

    if (m_is_selecting)
    {
        draw_selection_box(m_selection_start, m_window.cursor());
    }
}

void Graph::draw_selection_box(const glm::dvec2 &start, const glm::dvec2 &end) const
{
    m_line_renderer.draw_line(start, glm::dvec2(start.x, end.y), glm::vec3(1.0, 1.0, 1.0));
    m_line_renderer.draw_line(glm::dvec2(start.x, end.y), end, glm::vec3(1.0, 1.0, 1.0));
    m_line_renderer.draw_line(end, glm::dvec2(end.x, start.y), glm::vec3(1.0, 1.0, 1.0));
    m_line_renderer.draw_line(glm::dvec2(end.x, start.y), start, glm::vec3(1.0, 1.0, 1.0));
}

void Graph::init_line_buffers()
{
    glGenVertexArrays(1, &_linebuf_vao);
    glBindVertexArray(_linebuf_vao);

    glGenBuffers(1, &_linebuf_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 1024, nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("block/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("block/fragment.glsl"), GL_FRAGMENT_SHADER)};
    _lines_shader = Program(shaders);
}

void Graph::draw_lines()
{
    int offset = 0;

    const auto [tick_spacing_major, tick_spacing_minor, _] = tick_spacing();

    glBindVertexArray(_linebuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);

    // Get a pointer to the underlying buffer
    void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

    glm::ivec2 tl(GUTTER_SIZE_PX, 0);
    glm::ivec2 bl(GUTTER_SIZE_PX, m_size.y - GUTTER_SIZE_PX);
    glm::ivec2 br(m_size.x, m_size.y - GUTTER_SIZE_PX);

    // Draw the y axis line
    ptr[offset++] = tl;
    ptr[offset++] = bl;

    // Draw the x axis line
    ptr[offset++] = bl;
    ptr[offset++] = br;

    auto draw_ticks = [&](const glm::dvec2 &tick_spacing,
                          const glm::ivec2 &tick_size_y,
                          const glm::ivec2 &tick_size_x) {
        // Draw the y-axis ticks
        // Work out where (in graph space) margin and height-margin is
        auto top_gs = screen2graph(tl);
        auto bottom_gs = screen2graph(bl);
        double start = ceil(bottom_gs.y / tick_spacing.y) * tick_spacing.y;
        double end = ceil(top_gs.y / tick_spacing.y) * tick_spacing.y;

        // Place a tick at every unit up the y axis
        for (double i = start; i < end; i += tick_spacing.y)
        {
            auto tick_y_vpspace =
                m_window.vp_matrix() * (m_state.view_matrix * glm::dvec3(0.0f, i, 1.0f));
            ptr[offset++] = glm::vec2(GUTTER_SIZE_PX, tick_y_vpspace.y) + glm::vec2(tick_size_y);
            ptr[offset++] = glm::vec2(GUTTER_SIZE_PX, tick_y_vpspace.y);
        }

        // Draw the y axis ticks
        auto left_gs = screen2graph(bl);
        auto right_gs = screen2graph(br);
        start = ceil(left_gs.x / tick_spacing.x) * tick_spacing.x;
        end = ceil(right_gs.x / tick_spacing.x) * tick_spacing.x;

        // Place a tick at every unit along the x axis
        for (double i = start; i < end; i += tick_spacing.x)
        {
            auto tick_x_vpspace =
                m_window.vp_matrix() * (m_state.view_matrix * glm::dvec3(i, 0.0f, 1.0f));
            ptr[offset++] =
                glm::vec2(tick_x_vpspace.x, m_size.y - GUTTER_SIZE_PX) + glm::vec2(tick_size_x);
            ptr[offset++] = glm::vec2(tick_x_vpspace.x, m_size.y - GUTTER_SIZE_PX);
        }
    };

    draw_ticks(tick_spacing_major, glm::dvec2(-TICKLEN_PX, 0), glm::dvec2(0, TICKLEN_PX));
    draw_ticks(tick_spacing_minor, glm::dvec2(-TICKLEN_PX / 2, 0), glm::dvec2(0, TICKLEN_PX / 2));

    // // Add one additional vertical line where the cursor is
    // const auto cursor = m_window.cursor();
    // if (hit_test(cursor, glm::ivec2(GUTTER_SIZE_PX, m_size.y - GUTTER_SIZE_PX), m_size))
    // {
    //     ptr[offset++] = glm::vec2(cursor.x, 0.0);
    //     ptr[offset++] = glm::vec2(cursor.x, m_size.y);
    // }
    // else if (hit_test(
    //              cursor, glm::ivec2(0), glm::ivec2(GUTTER_SIZE_PX, m_size.y -
    //              GUTTER_SIZE_PX)))
    // {
    //     ptr[offset++] = glm::vec2(0.0, cursor.y);
    //     ptr[offset++] = glm::vec2(m_size.x, cursor.y);
    // }

    auto line_round = [](float value) { return roundf(value - 0.5f) + 0.5f; };
    for (int i = 0; i < offset; i++)
    {
        ptr[i].x = line_round(ptr[i].x);
        ptr[i].y = line_round(ptr[i].y);
    }

    // make sure to tell OpenGL we're done with the pointer
    glUnmapBuffer(GL_ARRAY_BUFFER);

    _lines_shader.use();
    int uniform_id = _lines_shader.uniform_location("view_matrix");
    const auto viewport_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(viewport_matrix_inv[0]));

    uniform_id = _lines_shader.uniform_location("colour");
    glm::vec3 white(1.0, 1.0, 1.0);
    glUniform3fv(uniform_id, 1, &white[0]);

    glDrawArrays(GL_LINES, 0, offset);
}

void Graph::draw_labels()
{
    glm::ivec2 tl(GUTTER_SIZE_PX, 0);
    glm::ivec2 bl(GUTTER_SIZE_PX, m_size.y - GUTTER_SIZE_PX);
    glm::ivec2 br(m_size.x, m_size.y - GUTTER_SIZE_PX);

    auto [tick_spacing_major, _, precision] = tick_spacing();

    // Draw one label per tick on the y axis
    auto top_gs = screen2graph(tl);
    auto bottom_gs = screen2graph(bl);
    double start = ceil(bottom_gs.y / tick_spacing_major.y) * tick_spacing_major.y;
    double end = ceil(top_gs.y / tick_spacing_major.y) * tick_spacing_major.y;

    std::size_t label_offset = 0;

    // Place a tick at every unit up the y axis
    for (double i = start; i < end; i += tick_spacing_major.y)
    {
        auto tick_y_vpspace =
            m_window.vp_matrix() * (m_state.view_matrix * glm::dvec3(0.0f, i, 1.0f));
        glm::ivec2 point(GUTTER_SIZE_PX - TICKLEN_PX, tick_y_vpspace.y);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.y) << i;

        auto &label = m_axis_labels[label_offset++];
        label.set_text(ss.str());
        label.set_position(point);
        label.set_alignment(Label::AlignmentHorizontal::Right);
        label.set_alignment(Label::AlignmentVertical::Center);
        label.draw();
    }

    // Draw the y axis ticks
    auto left_gs = screen2graph(bl);
    auto right_gs = screen2graph(br);
    start = ceil(left_gs.x / tick_spacing_major.x) * tick_spacing_major.x;
    end = ceil(right_gs.x / tick_spacing_major.x) * tick_spacing_major.x;

    // Place a tick at every unit along the x axis
    for (double i = start; i < end; i += tick_spacing_major.x)
    {
        auto tick_x_vpspace =
            m_window.vp_matrix() * (m_state.view_matrix * glm::dvec3(i, 0.0f, 1.0f));
        glm::ivec2 point(tick_x_vpspace.x, m_size.y - GUTTER_SIZE_PX + TICKLEN_PX);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.x) << i;

        auto &label = m_axis_labels[label_offset++];
        label.set_text(ss.str());
        label.set_position(point);
        label.set_alignment(Label::AlignmentHorizontal::Center);
        label.set_alignment(Label::AlignmentVertical::Top);
        label.draw();
    }

    // if (hit_test(m_window.cursor(),
    //              glm::ivec2(GUTTER_SIZE_PX, m_size.y - GUTTER_SIZE_PX),
    //              glm::ivec2(m_size.x, m_size.y)))
    // {
    //     // Find the nearest sample and draw labels for it
    //     auto cursor_gs =
    //         glm::inverse(_view_matrix) * m_window.vp_matrix_inv() * glm::vec3(cursor, 1.0f);
    //     auto cursor_gs2 = glm::inverse(_view_matrix) * m_window.vp_matrix_inv() *
    //                       glm::vec3(cursor.x + 1.0f, _cursor.y, 1.0f);

    //     for (const auto &ts : time_series)
    //     {
    //         if (ts.visible)
    //         {
    //             auto sample = ts.ts->get_sample(cursor_gs.x, cursor_gs2.x - cursor_gs.x);

    //             const auto draw_label = [&](double value) {
    //                 glm::vec2 sample_gs(cursor_gs.x, value);

    //                 glm::vec3 point3 =
    //                     m_window.vp_matrix() * _view_matrix * glm::vec3(sample_gs, 1.0f);
    //                 glm::vec2 point(point3.x, point3.y);

    //                 std::stringstream ss;
    //                 ss << value;
    //                 _draw_label(ss.str(),
    //                             point,
    //                             18,
    //                             7,
    //                             Alignment::Right,
    //                             AlignmentVertical::Center);
    //             };

    //             draw_label(sample.average);
    //             // draw_label(sample.min);
    //             // draw_label(sample.max);
    //         }
    //     }
    // }
}

void Graph::draw_plots()
{
    const auto graph_size_px = m_window.size();
    const auto plot_size_px = graph_size_px - glm::ivec2(GUTTER_SIZE_PX, GUTTER_SIZE_PX);
    const auto plot_position_px = glm::ivec2(GUTTER_SIZE_PX, 0);

    const int num_samples = plot_size_px.x / PIXELS_PER_COL;

    const auto plot_position_gs = screen2graph(plot_position_px);
    const auto plot_size_gs = screen2graph_delta(plot_size_px);
    const auto interval_gs = PIXELS_PER_COL * plot_size_gs.x / num_samples;

    std::size_t plot_index = 0;

    for (auto &time_series : m_state.timeseries)
    {
        if (time_series.visible)
        {
            std::vector<TSSample> samples(num_samples);
            auto n_samples = time_series.ts->get_samples(
                samples.data(), plot_position_gs.x, interval_gs, num_samples);
            samples.resize(n_samples);

            auto &plot = m_plots[plot_index++];
            plot.set_position(glm::ivec2(GUTTER_SIZE_PX, 0));
            plot.set_size(m_size - glm::ivec2(GUTTER_SIZE_PX, GUTTER_SIZE_PX));
            plot.draw(m_state.view_matrix,
                      samples,
                      m_state.plot_width,
                      time_series.colour,
                      time_series.y_offset,
                      m_state.show_line_segments);
        }
    }
}

void Graph::draw_markers()
{
    // auto marker_styles = [this] {
    //     const auto both_markers_visible =
    //         m_state.markers.first.visible && m_state.markers.second.visible;
    //     if (!both_markers_visible)
    //     {
    //         return std::make_pair(Marker::MarkerStyle::Center, Marker::MarkerStyle::Center);
    //     }

    //     if (m_state.markers.first.position < m_state.markers.second.position)
    //     {
    //         return std::make_pair(Marker::MarkerStyle::Left, Marker::MarkerStyle::Right);
    //     }
    //     else
    //     {
    //         return std::make_pair(Marker::MarkerStyle::Right, Marker::MarkerStyle::Left);
    //     }
    // }();

    const auto graph_size_px = m_window.size();
    const auto plot_size_px = graph_size_px - glm::ivec2(GUTTER_SIZE_PX, GUTTER_SIZE_PX);

    const int num_samples = plot_size_px.x / PIXELS_PER_COL;

    const auto plot_size_gs = screen2graph_delta(plot_size_px);
    const auto interval_gs = PIXELS_PER_COL * plot_size_gs.x / num_samples;

    std::size_t marker_ts_label_index = 0;

    if (m_state.markers.first.visible)
    {
        glm::ivec2 marker_pos = glm::dmat3(m_window.vp_matrix()) * m_state.view_matrix *
                                glm::dvec3(m_state.markers.first.position, 0.0, 1.0);
        marker_pos.y = 0;

        auto [_1, _2, precision] = tick_spacing();
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.x + 2) << m_state.markers.first.position;

        m_marker_a.set_label_text(ss.str());
        m_marker_a.set_height(m_size.y - GUTTER_SIZE_PX);
        m_marker_a.set_position(marker_pos);
        m_marker_a.draw();

        for (const auto &time_series : m_state.timeseries)
        {
            if (time_series.visible)
            {
                const auto value =
                    time_series.ts->get_sample(m_state.markers.first.position, interval_gs);

                std::stringstream ss;
                ss << std::fixed << std::setprecision(precision.y + 2) << value.average;

                auto &label = m_marker_ts_labels[marker_ts_label_index++];
                label.set_colour(time_series.colour);
                label.set_alignment(Label::AlignmentHorizontal::Left);
                label.set_alignment(Label::AlignmentVertical::Top);
                label.set_position(glm::dmat3(m_window.vp_matrix()) * m_state.view_matrix *
                                   glm::dvec3(m_state.markers.first.position,
                                              value.average + time_series.y_offset,
                                              1.0));
                label.set_text(ss.str());
                label.draw();
            }
        }
    }

    if (m_state.markers.second.visible)
    {
        glm::ivec2 marker_pos = glm::dmat3(m_window.vp_matrix()) * m_state.view_matrix *
                                glm::dvec3(m_state.markers.second.position, 0.0, 1.0);
        marker_pos.y = 0;

        auto [_1, _2, precision] = tick_spacing();
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.x + 2) << m_state.markers.second.position;

        m_marker_b.set_label_text(ss.str());
        m_marker_b.set_height(m_size.y - GUTTER_SIZE_PX);
        m_marker_b.set_position(marker_pos);
        m_marker_b.draw();

        for (const auto &time_series : m_state.timeseries)
        {
            if (time_series.visible)
            {
                const auto value =
                    time_series.ts->get_sample(m_state.markers.second.position, interval_gs);

                std::stringstream ss;
                ss << std::fixed << std::setprecision(precision.y + 2) << value.average;

                auto &label = m_marker_ts_labels[marker_ts_label_index++];
                label.set_colour(time_series.colour);
                label.set_alignment(Label::AlignmentHorizontal::Left);
                label.set_alignment(Label::AlignmentVertical::Top);
                label.set_position(glm::dmat3(m_window.vp_matrix()) * m_state.view_matrix *
                                   glm::dvec3(m_state.markers.second.position,
                                              value.average + time_series.y_offset,
                                              1.0));
                label.set_text(ss.str());
                label.draw();
            }
        }
    }
}

std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> Graph::tick_spacing() const
{
    const glm::dvec2 MIN_TICK_SPACING_PX(80, 50);

    // Calc the size of this vector in graph space (ignoring translation & sign)
    const glm::dvec2 min_tick_spacing_gs =
        glm::abs(screen2graph(glm::dvec2(0.0f, 0.0f)) - screen2graph(MIN_TICK_SPACING_PX));

    // Round this size up to the nearest power of 10
    glm::dvec2 tick_spacing;
    tick_spacing.x = pow(10.0f, ceil(log10(min_tick_spacing_gs.x)));
    tick_spacing.y = pow(10.0f, ceil(log10(min_tick_spacing_gs.y)));
    glm::dvec2 minor_tick_spacing = tick_spacing / 2.0;

    glm::ivec2 precision;
    precision.x = -ceil(log10(min_tick_spacing_gs.x));
    precision.y = -ceil(log10(min_tick_spacing_gs.y));

    auto scale = min_tick_spacing_gs / tick_spacing;
    if (scale.x < 0.5f)
    {
        tick_spacing.x /= 2;
        ++precision.x;
        minor_tick_spacing.x = tick_spacing.x / 5;
    }

    if (scale.y < 0.5f)
    {
        tick_spacing.y /= 2;
        ++precision.y;
        minor_tick_spacing.y = tick_spacing.y / 5;
    }

    if (precision.x < 0)
        precision.x = 0;
    if (precision.y < 0)
        precision.y = 0;

    return std::tuple(tick_spacing, minor_tick_spacing, precision);
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

    if (m_marker_a.is_dragging)
    {
        m_state.markers.first.position = m_state.markers.first.position + cursor_gs_delta.x;
    }

    if (m_marker_b.is_dragging)
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
            m_marker_a.is_dragging = true;
        }
        else if (marker_clicked(m_state.markers.second))
        {
            m_marker_b.is_dragging = true;
        }
        else
        {
            m_is_dragging = true;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        m_is_dragging = false;
        m_marker_a.is_dragging = false;
        m_marker_b.is_dragging = false;
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
