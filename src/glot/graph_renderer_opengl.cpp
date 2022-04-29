
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <sstream>
#include <iomanip>
#include <stb_image/stb_image.h>

#include "graph_renderer_opengl.hpp"
#include "resources.hpp"
#include "text_renderer_opengl.hpp"

GraphRendererOpenGL::GraphRendererOpenGL(Window &window)
    : m_window(window), m_plot(window), m_font_material("proggy_clean.png"),
      m_text_renderer(m_window, m_font_material), m_marker_a(window), m_marker_b(window),
      m_line_renderer(m_window), m_view_matrix(1.0), m_view_matrix_inv(1.0), m_gutter_size_px(60),
      m_tick_len_px(5)
{
    init_line_buffers();

    for (int i = 0; i < 128; ++i)
    {
        m_axis_labels.emplace_back(m_window, m_font_material);
        m_axis_labels.back().set_colour(glm::vec3(1.0, 1.0, 1.0));
    }
}

GraphRendererOpenGL::~GraphRendererOpenGL()
{
    glDeleteVertexArrays(1, &_linebuf_vao);
    glDeleteBuffers(1, &_linebuf_vbo);
}

void GraphRendererOpenGL::set_view_matrix(const glm::dmat3 &view_matrix)
{
    m_view_matrix = view_matrix;
    m_view_matrix_inv = glm::inverse(view_matrix);
}

void GraphRendererOpenGL::set_size(const glm::ivec2 &size)
{
    m_size = size;
}

void GraphRendererOpenGL::set_gutter_size(int gutter_size_px)
{
    m_gutter_size_px = gutter_size_px;
}

void GraphRendererOpenGL::set_tick_len(int tick_len_px)
{
    m_tick_len_px = tick_len_px;
}

void GraphRendererOpenGL::draw_graph()
{
    draw_lines();
    draw_labels();
}

void GraphRendererOpenGL::draw_plot(const std::vector<TSSample> &data,
                                    int plot_width,
                                    glm::vec3 plot_colour,
                                    float y_offset,
                                    bool show_line_segments) const
{
    m_plot.draw(m_view_matrix,
                data,
                plot_width,
                plot_colour,
                y_offset,
                show_line_segments,
                glm::ivec2(m_gutter_size_px, 0),
                glm::ivec2(m_size.x - m_gutter_size_px, m_size.y - m_gutter_size_px));
}

void GraphRendererOpenGL::draw_marker(double position,
                                      Marker::MarkerStyle style,
                                      const glm::vec3 &colour)
{
    (void)style;
    auto pos_pixels = m_window.vp_matrix() * (m_view_matrix * glm::dvec3(position, 0.0, 1.0));
    pos_pixels = round(pos_pixels - 0.5f) + 0.5f;
    m_marker_renderer.set_position(glm::ivec2(pos_pixels.x, 0.0));
    m_marker_renderer.set_colour(colour);
    m_marker_renderer.set_height(m_window.size().y - m_gutter_size_px);
    m_marker_renderer.draw();

    // Draw the label which shows the x value of the marker
    auto [_1, _2, precision] = tick_spacing();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision.x + 2) << position;

    m_text_renderer.set_text(ss.str());
    m_text_renderer.set_colour(colour);
    m_text_renderer.set_position(glm::ivec2(pos_pixels.x, m_size.y - m_gutter_size_px / 2));
    m_text_renderer.draw_text(Label::LabelAlignmentHorizontal::Center,
                              Label::LabelAlignmentVertical::Top);
}

void GraphRendererOpenGL::draw_value_label(const glm::dvec2 &position,
                                           double value,
                                           const glm::vec3 &colour)
{
    auto pos_pixels = m_window.vp_matrix() * (m_view_matrix * glm::dvec3(position, 1.0));
    pos_pixels = round(pos_pixels - 0.5f) + 0.5f;

    auto [_1, _2, precision] = tick_spacing();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision.y + 2) << value;
    const auto label_text = ss.str();
    m_text_renderer.set_text(label_text);
    m_text_renderer.set_colour(colour);
    m_text_renderer.set_position(pos_pixels);
    m_text_renderer.draw_text(Label::LabelAlignmentHorizontal::Left,
                              Label::LabelAlignmentVertical::Top);
}

void GraphRendererOpenGL::draw_selection_box(const glm::dvec2 &start, const glm::dvec2 &end) const
{
    m_line_renderer.draw_line(start, glm::dvec2(start.x, end.y), glm::vec3(1.0, 1.0, 1.0));
    m_line_renderer.draw_line(glm::dvec2(start.x, end.y), end, glm::vec3(1.0, 1.0, 1.0));
    m_line_renderer.draw_line(end, glm::dvec2(end.x, start.y), glm::vec3(1.0, 1.0, 1.0));
    m_line_renderer.draw_line(glm::dvec2(end.x, start.y), start, glm::vec3(1.0, 1.0, 1.0));
}

void GraphRendererOpenGL::init_line_buffers()
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

void GraphRendererOpenGL::draw_lines() const
{
    int offset = 0;

    const auto [tick_spacing_major, tick_spacing_minor, _] = tick_spacing();

    glBindVertexArray(_linebuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);

    // Get a pointer to the underlying buffer
    void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

    glm::ivec2 tl(m_gutter_size_px, 0);
    glm::ivec2 bl(m_gutter_size_px, m_size.y - m_gutter_size_px);
    glm::ivec2 br(m_size.x, m_size.y - m_gutter_size_px);

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
                m_window.vp_matrix() * (m_view_matrix * glm::dvec3(0.0f, i, 1.0f));
            ptr[offset++] = glm::vec2(m_gutter_size_px, tick_y_vpspace.y) + glm::vec2(tick_size_y);
            ptr[offset++] = glm::vec2(m_gutter_size_px, tick_y_vpspace.y);
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
                m_window.vp_matrix() * (m_view_matrix * glm::dvec3(i, 0.0f, 1.0f));
            ptr[offset++] =
                glm::vec2(tick_x_vpspace.x, m_size.y - m_gutter_size_px) + glm::vec2(tick_size_x);
            ptr[offset++] = glm::vec2(tick_x_vpspace.x, m_size.y - m_gutter_size_px);
        }
    };

    draw_ticks(tick_spacing_major, glm::dvec2(-m_tick_len_px, 0), glm::dvec2(0, m_tick_len_px));
    draw_ticks(
        tick_spacing_minor, glm::dvec2(-m_tick_len_px / 2, 0), glm::dvec2(0, m_tick_len_px / 2));

    // // Add one additional vertical line where the cursor is
    // const auto cursor = m_window.cursor();
    // if (hit_test(cursor, glm::ivec2(m_gutter_size_px, m_size.y - m_gutter_size_px), m_size))
    // {
    //     ptr[offset++] = glm::vec2(cursor.x, 0.0);
    //     ptr[offset++] = glm::vec2(cursor.x, m_size.y);
    // }
    // else if (hit_test(
    //              cursor, glm::ivec2(0), glm::ivec2(m_gutter_size_px, m_size.y -
    //              m_gutter_size_px)))
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

void GraphRendererOpenGL::draw_labels()
{
    glm::ivec2 tl(m_gutter_size_px, 0);
    glm::ivec2 bl(m_gutter_size_px, m_size.y - m_gutter_size_px);
    glm::ivec2 br(m_size.x, m_size.y - m_gutter_size_px);

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
        auto tick_y_vpspace = m_window.vp_matrix() * (m_view_matrix * glm::dvec3(0.0f, i, 1.0f));
        glm::ivec2 point(m_gutter_size_px - m_tick_len_px, tick_y_vpspace.y);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.y) << i;

        auto &label = m_axis_labels[label_offset++];
        label.set_text(ss.str());
        label.set_position(point);
        label.draw_text(Label::LabelAlignmentHorizontal::Right,
                        Label::LabelAlignmentVertical::Center);
    }

    // Draw the y axis ticks
    auto left_gs = screen2graph(bl);
    auto right_gs = screen2graph(br);
    start = ceil(left_gs.x / tick_spacing_major.x) * tick_spacing_major.x;
    end = ceil(right_gs.x / tick_spacing_major.x) * tick_spacing_major.x;

    // Place a tick at every unit along the x axis
    for (double i = start; i < end; i += tick_spacing_major.x)
    {
        auto tick_x_vpspace = m_window.vp_matrix() * (m_view_matrix * glm::dvec3(i, 0.0f, 1.0f));
        glm::ivec2 point(tick_x_vpspace.x, m_size.y - m_gutter_size_px + m_tick_len_px);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.x) << i;

        auto &label = m_axis_labels[label_offset++];
        label.set_text(ss.str());
        label.set_position(point);
        label.draw_text(Label::LabelAlignmentHorizontal::Center,
                        Label::LabelAlignmentVertical::Top);
    }

    // if (hit_test(m_window.cursor(),
    //              glm::ivec2(m_gutter_size_px, m_size.y - m_gutter_size_px),
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
    //                             LabelAlignment::Right,
    //                             LabelAlignmentVertical::Center);
    //             };

    //             draw_label(sample.average);
    //             // draw_label(sample.min);
    //             // draw_label(sample.max);
    //         }
    //     }
    // }
}

std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> GraphRendererOpenGL::tick_spacing() const
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

glm::dvec2 GraphRendererOpenGL::screen2graph(const glm::ivec2 &value) const
{
    const glm::dvec3 value3(value, 1.0f);
    return m_view_matrix_inv * (m_window.vp_matrix_inv() * value3);
}
