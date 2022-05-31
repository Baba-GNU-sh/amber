#include "axis.hpp"
#include "resources.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <boost/signals2.hpp>

AxisBase::AxisBase(const Window &window) : m_window(window), m_font("proggy_clean.png")
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

    // Initialize the labels
    for (size_t i = 0; i < NUM_LABELS; ++i)
    {
        m_labels.emplace_back(m_font);
        m_labels.back().set_colour(glm::vec3(1.0, 1.0, 1.0));
    }

    m_labels_used = 0;
}

AxisBase::~AxisBase()
{
    glDeleteBuffers(1, &m_linebuf_vao);
    glDeleteVertexArrays(1, &m_linebuf_vao);
}

template <> Axis<AxisVertical>::Axis(const Window &window) : AxisBase(window)
{
}

template <> Axis<AxisHorizontal>::Axis(const Window &window) : AxisBase(window)
{
}

void AxisBase::draw(const Window &window)
{
    draw_ticks(window);
    draw_labels(window);
}

void AxisBase::draw_ticks(const Window &window) const
{
    int offset = 0;

    glBindVertexArray(m_linebuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_linebuf_vbo);

    // Get a pointer to the underlying buffer
    void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

    const auto [tick_spacing_major, tick_spacing_minor, _] = tick_spacing();

    draw_ticks(tick_spacing_major, TICKLEN_PX, ptr, offset);
    draw_ticks(tick_spacing_minor, TICKLEN_PX / 2, ptr, offset);

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

template <>
void Axis<AxisHorizontal>::draw_ticks(const glm::dvec2 &tick_spacing,
                                      double tick_size,
                                      glm::vec2 *const ptr,
                                      int &offset) const
{
    const glm::dvec2 tick_size_vec(0, tick_size);

    // Work out the positions of the start and end of the axis in graph space
    const auto axis_start_gs = screen2graph(m_position);
    const auto axis_end_gs = screen2graph(m_position + glm::dvec2(m_size.x, 0.0));

    // Align the start and end to the nearest tick to work out where the first and last
    // ticks should go
    const auto first_tick = crush(axis_start_gs, tick_spacing, true);
    const auto last_tick = crush(axis_end_gs, tick_spacing, true);

    // Work out how many ticks we are going to draw
    const glm::ivec2 n_ticks = glm::abs(last_tick - first_tick) / tick_spacing;

    for (int tick = 0; tick < n_ticks.x; ++tick)
    {
        auto tick_pos = first_tick + (static_cast<double>(tick) * tick_spacing);
        auto tick_pos_gs = graph2screen(tick_pos);
        tick_pos_gs.y = m_position.y;

        ptr[offset++] = tick_pos_gs;
        ptr[offset++] = tick_pos_gs + tick_size_vec;
    }

    ptr[offset++] = m_position;
    ptr[offset++] = m_position + glm::dvec2(m_size.x, 0.0);
}

template <>
void Axis<AxisVertical>::draw_ticks(const glm::dvec2 &tick_spacing,
                                    double tick_size,
                                    glm::vec2 *const ptr,
                                    int &offset) const
{
    const glm::dvec2 tick_size_vec(-tick_size, 0.0);

    // Work out the positions of the start and end of the axis in graph space
    const auto axis_start_gs = screen2graph(m_position);
    const auto axis_end_gs = screen2graph(m_position + glm::dvec2(0.0, m_size.y));

    // Align the start and end to the nearest tick to work out where the first and last
    // ticks should go
    const auto first_tick = crush(axis_start_gs, tick_spacing, false);
    const auto last_tick = crush(axis_end_gs, tick_spacing, false);

    // Work out how many ticks we are going to draw
    const glm::ivec2 n_ticks = glm::abs(last_tick - first_tick) / tick_spacing;

    for (int tick = 0; tick < n_ticks.y; ++tick)
    {
        auto tick_pos = first_tick - (static_cast<double>(tick) * tick_spacing);
        auto tick_pos_gs = graph2screen(tick_pos);
        tick_pos_gs.x = m_position.x + m_size.x;

        ptr[offset++] = tick_pos_gs;
        ptr[offset++] = tick_pos_gs + tick_size_vec;
    }

    ptr[offset++] = m_position + glm::dvec2(m_size.x, 0.0);
    ptr[offset++] = m_position + m_size;
}

void AxisBase::draw_labels(const Window &window)
{
    for (size_t i = 0; i < m_labels_used; i++)
    {
        m_labels[i].draw(window);
    }
}

glm::dvec2 AxisBase::position() const
{
    return m_position;
}
void AxisBase::set_position(const glm::dvec2 &position)
{
    m_position = position;
    update_layout();
}

glm::dvec2 AxisBase::size() const
{
    return m_size;
}
void AxisBase::set_size(const glm::dvec2 &size)
{
    m_size = size;
    update_layout();
}

void AxisBase::set_graph_transform(const Transform<double> &t)
{
    m_graph_transform = t;
    update_layout();
}

void AxisBase::on_scroll(const glm::dvec2 &cursor_position, double, double yoffset)
{
    (void)cursor_position;
    on_zoom(yoffset);
}

void AxisBase::on_mouse_button(const glm::dvec2 &, int button, int action, int)
{
    spdlog::info("Click {} {}", button, action);
}

std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> AxisBase::tick_spacing() const
{
    // TODO: This needs to take into account the size of the axis... I think!?
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

glm::dvec2 AxisBase::screen2graph(const glm::dvec2 &viewport_space) const
{
    const auto clip_space = m_window.viewport_transform().apply_inverse(viewport_space);
    const auto graph_space = m_graph_transform.apply_inverse(clip_space);
    return graph_space;
}

glm::dvec2 AxisBase::screen2graph_delta(const glm::dvec2 &delta) const
{
    auto begin_gs = screen2graph(glm::dvec2(0, 0));
    auto end_gs = screen2graph(glm::dvec2(0, 0) + delta);
    return end_gs - begin_gs;
}

glm::dvec2 AxisBase::graph2screen(const glm::dvec2 &value) const
{
    const auto clip_space = m_graph_transform.apply(value);
    const auto screen_space = m_window.viewport_transform().apply(clip_space);
    return screen_space;
}

glm::dvec2 AxisBase::crush(const glm::dvec2 &value, const glm::dvec2 &interval, bool ceil)
{
    if (ceil)
    {
        return glm::dvec2(std::ceil(value.x / interval.x) * interval.x,
                          std::ceil(value.y / interval.y) * interval.y);
    }
    else
    {

        return glm::dvec2(std::floor(value.x / interval.x) * interval.x,
                          std::floor(value.y / interval.y) * interval.y);
    }
}

template <> void Axis<AxisHorizontal>::update_layout()
{
    const auto [label_spacing, _, label_precision] = tick_spacing();

    m_labels_used = 0;

    // Work out the positions of the start and end of the axis in graph space
    const auto axis_start_gs = screen2graph(m_position);
    const auto axis_end_gs = screen2graph(m_position + glm::dvec2(m_size.x, 0.0));

    // Align the start and end to the nearest tick to work out where the first and last ticks
    // should go
    const auto first_tick = crush(axis_start_gs, label_spacing, true);
    const auto last_tick = crush(axis_end_gs, label_spacing, true);

    // Work out how many ticks we are going to draw
    const glm::ivec2 n_ticks = glm::abs(last_tick - first_tick) / label_spacing;

    for (int tick = 0; tick < n_ticks.x; ++tick)
    {
        auto tick_pos = first_tick + (static_cast<double>(tick) * label_spacing);
        auto tick_pos_ss = graph2screen(tick_pos);
        tick_pos_ss.y = m_position.y;

        auto &label = m_labels[m_labels_used++];
        label.set_position(tick_pos_ss);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(label_precision.x) << tick_pos.x;
        label.set_text(ss.str());

        label.set_alignment(Label::AlignmentHorizontal::Center);
        label.set_alignment(Label::AlignmentVertical::Top);
    }
}

template <> void Axis<AxisVertical>::update_layout()
{
    const auto [label_spacing, _, label_precision] = tick_spacing();

    m_labels_used = 0;

    // Work out the positions of the start and end of the axis in graph space
    const auto axis_start_gs = screen2graph(m_position);
    const auto axis_end_gs = screen2graph(m_position + glm::dvec2(0.0, m_size.y));

    // Align the start and end to the nearest tick to work out where the first and last
    // ticks should go
    const auto first_tick = crush(axis_start_gs, label_spacing, false);
    const auto last_tick = crush(axis_end_gs, label_spacing, false);

    // Work out how many ticks we are going to draw
    const glm::ivec2 n_ticks = glm::abs(last_tick - first_tick) / label_spacing;

    for (int tick = 0; tick < n_ticks.y; ++tick)
    {
        auto tick_pos = first_tick - (static_cast<double>(tick) * label_spacing);
        auto tick_pos_ss = graph2screen(tick_pos);
        tick_pos_ss.x = m_position.x + m_size.x;

        auto &label = m_labels[m_labels_used++];
        label.set_position(tick_pos_ss);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(label_precision.y) << tick_pos.y;
        label.set_text(ss.str());

        label.set_alignment(Label::AlignmentHorizontal::Right);
        label.set_alignment(Label::AlignmentVertical::Center);
    }
}
