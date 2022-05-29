#include "axis.hpp"
#include "resources.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <boost/signals2.hpp>

Axis::Axis(Orientation ori, const Transform<double> &graph_transform)
    : m_orientation(ori), m_graph_transform(graph_transform), m_font("proggy_clean.png")
{
    glGenVertexArrays(1, &m_linebuf_vao);
    glBindVertexArray(m_linebuf_vao);

    glGenBuffers(1, &m_linebuf_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_linebuf_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 1024, nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("block/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("block/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_lines_shader = Program(shaders);

    // TODO: Lose the magic number here
    for (int i = 0; i < 128; ++i)
    {
        m_axis_labels.emplace_back(m_font);
        m_axis_labels.back().set_colour(glm::vec3(1.0, 1.0, 1.0));
    }
}

Axis::~Axis()
{
    glDeleteBuffers(1, &m_linebuf_vao);
    glDeleteVertexArrays(1, &m_linebuf_vao);
}

void Axis::draw(const Window &window) const
{
    draw_ticks(window);
    draw_labels(window);
}

void Axis::draw_ticks(const Window &window) const
{
    const auto &vpt = window.viewport_transform();
    int offset = 0;

    glBindVertexArray(m_linebuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_linebuf_vbo);

    // Get a pointer to the underlying buffer
    void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

    const auto [tick_spacing_major, tick_spacing_minor, _] = tick_spacing(vpt);

    draw_ticks(tick_spacing_major, TICKLEN_PX, ptr, offset, vpt);
    draw_ticks(tick_spacing_minor, TICKLEN_PX / 2, ptr, offset, vpt);

    if (m_orientation == Orientation::Horizontal)
    {
        ptr[offset++] = m_position;
        ptr[offset++] = m_position + glm::dvec2(m_size.x, 0.0);
    }
    else
    {
        ptr[offset++] = m_position + glm::dvec2(m_size.x, 0.0);
        ptr[offset++] = m_position + m_size;
    }

    // make sure to tell OpenGL we're done with the pointer
    glUnmapBuffer(GL_ARRAY_BUFFER);

    m_lines_shader.use();
    int uniform_id = m_lines_shader.uniform_location("view_matrix");
    const auto viewport_matrix_inv = glm::mat3(window.viewport_transform().matrix_inverse());
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(viewport_matrix_inv[0]));

    uniform_id = m_lines_shader.uniform_location("colour");
    glm::vec3 white(1.0, 1.0, 1.0);
    glUniform3fv(uniform_id, 1, &white[0]);

    glDrawArrays(GL_LINES, 0, offset);
}

void Axis::draw_ticks(const glm::dvec2 &tick_spacing,
                      double tick_size,
                      glm::vec2 *const ptr,
                      int &offset,
                      const Transform<double> &vpt) const
{
    if (m_orientation == Orientation::Horizontal)
    {
        const glm::dvec2 tick_size_vec(0, tick_size);

        // Work out the positions of the start and end of the axis in graph space
        const auto axis_start_gs = screen2graph(vpt, m_position);
        const auto axis_end_gs = screen2graph(vpt, m_position + glm::dvec2(m_size.x, 0.0));

        // Align the start and end to the nearest tick to work out where the first and last
        // ticks should go
        const auto first_tick = crush(axis_start_gs, tick_spacing);
        const auto last_tick = crush(axis_end_gs, tick_spacing);

        // Work out how many ticks we are going to draw
        const glm::ivec2 n_ticks = glm::abs(last_tick - first_tick) / tick_spacing;

        for (int tick = 0; tick < n_ticks.x; ++tick)
        {
            auto tick_pos = first_tick + (static_cast<double>(tick) * tick_spacing);
            auto tick_pos_gs = graph2screen(vpt, tick_pos);
            tick_pos_gs.y = m_position.y;

            ptr[offset++] = tick_pos_gs;
            ptr[offset++] = tick_pos_gs + tick_size_vec;
        }
    }
    else
    {
        const glm::dvec2 tick_size_vec(-tick_size, 0.0);

        // Work out the positions of the start and end of the axis in graph space
        const auto axis_start_gs = screen2graph(vpt, m_position);
        const auto axis_end_gs = screen2graph(vpt, m_position + glm::dvec2(0.0, m_size.y));

        // Align the start and end to the nearest tick to work out where the first and last
        // ticks should go
        const auto first_tick = crush(axis_start_gs, tick_spacing);
        const auto last_tick = crush(axis_end_gs, tick_spacing);

        // Work out how many ticks we are going to draw
        const glm::ivec2 n_ticks = glm::abs(last_tick - first_tick) / tick_spacing;

        for (int tick = 0; tick < n_ticks.y; ++tick)
        {
            auto tick_pos = first_tick - (static_cast<double>(tick) * tick_spacing);
            auto tick_pos_gs = graph2screen(vpt, tick_pos);
            tick_pos_gs.x = m_position.x + m_size.x;

            ptr[offset++] = tick_pos_gs;
            ptr[offset++] = tick_pos_gs + tick_size_vec;
        }
    }
}

void Axis::draw_labels(const Window &window)
{
    const auto vpt = window.viewport_transform();

    glm::ivec2 tl(GUTTER_SIZE_PX, 0);
    glm::ivec2 bl(GUTTER_SIZE_PX, m_size.y - GUTTER_SIZE_PX);
    glm::ivec2 br(m_size.x, m_size.y - GUTTER_SIZE_PX);

    auto [tick_spacing_major, _, precision] = tick_spacing(vpt);

    // Draw one label per tick on the y axis
    auto top_gs = screen2graph(vpt, tl);
    auto bottom_gs = screen2graph(vpt, bl);
    double start = ceil(bottom_gs.y / tick_spacing_major.y) * tick_spacing_major.y;
    double end = ceil(top_gs.y / tick_spacing_major.y) * tick_spacing_major.y;

    std::size_t label_offset = 0;

    // Place a tick at every unit up the y axis
    for (double i = start; i < end; i += tick_spacing_major.y)
    {
        auto tick_y_vpspace = vpt.apply(m_graph_transform.apply(glm::dvec2(0.0f, i)));
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
    auto left_gs = screen2graph(vpt, bl);
    auto right_gs = screen2graph(vpt, br);
    start = ceil(left_gs.x / tick_spacing_major.x) * tick_spacing_major.x;
    end = ceil(right_gs.x / tick_spacing_major.x) * tick_spacing_major.x;

    // Place a tick at every unit along the x axis
    for (double i = start; i < end; i += tick_spacing_major.x)
    {
        auto tick_x_vpspace =
            window.viewport_transform().apply(m_graph_transform.apply(glm::dvec2(i, 0.0f)));
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

void Axis::draw_labels(const Window &window) const
{
    const auto &vpt = window.viewport_transform();

    const auto [tick_spacing_major, _, precision] = tick_spacing(vpt);

    (void)tick_spacing_major;
    (void)precision;

    if (m_orientation == Orientation::Horizontal)
    {
    }
    else
    {
    }
}

glm::dvec2 Axis::position() const
{
    return m_position;
}
void Axis::set_position(const glm::dvec2 &position)
{
    m_position = position;
}

glm::dvec2 Axis::size() const
{
    return m_size;
}
void Axis::set_size(const glm::dvec2 &size)
{
    m_size = size;
}

void Axis::set_orientation(Orientation ori)
{
    m_orientation = ori;
}

void Axis::on_scroll(Window &window, double, double yoffset)
{
    on_zoom(window, yoffset);
}

void Axis::on_mouse_button(Window &, int button, int action, int)
{
    spdlog::info("Click {} {}", button, action);
}

std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> Axis::tick_spacing(
    const Transform<double> &viewport_transform) const
{
    // TODO: This needs to take into account the size of the axis... I think!?
    const glm::dvec2 MIN_TICK_SPACING_PX(80, 50);

    // Calc the size of this vector in graph space (ignoring translation & sign)
    const glm::dvec2 min_tick_spacing_gs =
        glm::abs(screen2graph(viewport_transform, glm::dvec2(0.0f, 0.0f)) -
                 screen2graph(viewport_transform, MIN_TICK_SPACING_PX));

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

glm::dvec2 Axis::screen2graph(const Transform<double> &viewport_txform,
                              const glm::ivec2 &viewport_space) const
{
    const auto clip_space = viewport_txform.apply_inverse(viewport_space);
    const auto graph_space = m_graph_transform.apply_inverse(clip_space);
    return graph_space;
}

glm::dvec2 Axis::screen2graph_delta(const Transform<double> &viewport_txform,
                                    const glm::ivec2 &delta) const
{
    auto begin_gs = screen2graph(viewport_txform, glm::ivec2(0, 0));
    auto end_gs = screen2graph(viewport_txform, glm::ivec2(0, 0) + delta);
    return end_gs - begin_gs;
}

glm::dvec2 Axis::graph2screen(const Transform<double> &viewport_txform,
                              const glm::dvec2 &value) const
{
    const auto clip_space = m_graph_transform.apply(value);
    const auto screen_space = viewport_txform.apply(clip_space);
    return screen_space;
}

glm::dvec2 Axis::crush(const glm::dvec2 &value, const glm::dvec2 &interval)
{
    return glm::dvec2(ceil(value.x / interval.x) * interval.x,
                      ceil(value.y / interval.y) * interval.y);
}
