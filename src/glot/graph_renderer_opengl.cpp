
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <sstream>
#include <iomanip>
#include <stb_image/stb_image.h>

#include "graph_renderer_opengl.hpp"
#include "resources.hpp"
#include "label.hpp"

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
