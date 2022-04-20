
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <sstream>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "graph_renderer_opengl.hpp"
#include "resources.hpp"

GraphRendererOpenGL::GraphRendererOpenGL(Window &window)
    : m_window(window), m_plot(window), m_view_matrix(1.0), m_view_matrix_inv(1.0),
      m_gutter_size_px(60), m_tick_len_px(5)
{
    init_line_buffers();
    init_glyph_buffers();
}

GraphRendererOpenGL::~GraphRendererOpenGL()
{
    glDeleteVertexArrays(1, &_linebuf_vao);
    glDeleteBuffers(1, &_linebuf_vbo);
    glDeleteVertexArrays(1, &_glyphbuf_vao);
    glDeleteBuffers(1, &_glyphbuf_vbo);
    glDeleteBuffers(1, &_glyphbuf_ebo);
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

void GraphRendererOpenGL::draw_graph() const
{
    draw_lines();
    draw_labels();
}

void GraphRendererOpenGL::draw_plot(const TimeSeries &ts,
                                    int plot_width,
                                    glm::vec3 plot_colour,
                                    float y_offset,
                                    bool show_line_segments) const
{
    m_plot.draw(m_view_matrix,
                ts,
                plot_width,
                plot_colour,
                y_offset,
                show_line_segments,
                glm::ivec2(m_gutter_size_px, 0),
                glm::ivec2(m_size.x - m_gutter_size_px, m_size.y - m_gutter_size_px));
}

void GraphRendererOpenGL::draw_marker(const std::string &label,
                                      double position,
                                      MarkerStyle style,
                                      const glm::vec3 &colour) const
{
    (void)label;
    (void)style;

    // Store the marker's info away for later
    int offset = 0;

    glBindVertexArray(_linebuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);

    // Get a pointer to the underlying buffer
    void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

    auto pos_pixels = m_window.vp_matrix() * (m_view_matrix * glm::dvec3(position, 0.0, 1.0));
    pos_pixels = round(pos_pixels - 0.5f) + 0.5f;

    ptr[offset++] = glm::vec2(pos_pixels.x, 0.0);
    ptr[offset++] = glm::vec2(pos_pixels.x, m_size.y - m_gutter_size_px);

    // make sure to tell OpenGL we're done with the pointer
    glUnmapBuffer(GL_ARRAY_BUFFER);

    _lines_shader.use();
    int uniform_id = _lines_shader.uniform_location("view_matrix");
    const auto viewport_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(viewport_matrix_inv[0]));

    uniform_id = _lines_shader.uniform_location("line_colour");
    glUniform3fv(uniform_id, 1, &colour[0]);
    glDrawArrays(GL_LINES, 0, offset);

    auto [_1, _2, precision] = tick_spacing();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision.x + 2) << position;
    _draw_label(ss.str(),
                glm::ivec2(pos_pixels.x, m_size.y - m_gutter_size_px / 2),
                18,
                7,
                LabelAlignment::Center,
                LabelAlignmentVertical::Top,
                colour);
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
        Shader(Resources::find_shader("line/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("line/fragment.glsl"), GL_FRAGMENT_SHADER)};
    _lines_shader = Program(shaders);
}

void GraphRendererOpenGL::init_glyph_buffers()
{
    glGenVertexArrays(1, &_glyphbuf_vao);
    glBindVertexArray(_glyphbuf_vao);

    glGenBuffers(1, &_glyphbuf_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _glyphbuf_vbo);

    glGenBuffers(1, &_glyphbuf_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glyphbuf_ebo);

    const int sz = 6 * 128 / 4;
    unsigned int indices[sz];
    for (int i = 0, j = 0; j < sz; i += 4, j += 6)
    {
        indices[j] = i;
        indices[j + 1] = i + 1;
        indices[j + 2] = i + 2;
        indices[j + 3] = i + 1;
        indices[j + 4] = i + 2;
        indices[j + 5] = i + 3;
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // A glyph is rendered as a quad so we only need 4 verts and 4 texture
    // lookups
    glBufferData(GL_ARRAY_BUFFER, 128 * sizeof(GlyphData), nullptr, GL_STREAM_DRAW);

    // Define an attribute for the glyph verticies
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void *)0);
    glEnableVertexAttribArray(0);

    // Define an attribute for the texture lookups
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void *)offsetof(GlyphVertex, tex_coords));
    glEnableVertexAttribArray(1);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("glyph/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("glyph/fragment.glsl"), GL_FRAGMENT_SHADER)};
    _glyph_shader = Program(shaders);

    int width, height, nrChannels;
    unsigned char *tex_data = stbi_load(
        Resources::find_font("proggy_clean.png").c_str(), &width, &height, &nrChannels, 0);
    if (!tex_data)
    {
        throw std::runtime_error("Unable to load font map: " + std::string(stbi_failure_reason()));
    }

    glGenTextures(1, &_glyph_texture);
    glBindTexture(GL_TEXTURE_2D, _glyph_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glGenerateMipmap(GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, tex_data);

    stbi_image_free(tex_data);
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
    //              cursor, glm::ivec2(0), glm::ivec2(m_gutter_size_px, m_size.y - m_gutter_size_px)))
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

    uniform_id = _lines_shader.uniform_location("line_colour");
    glm::vec3 white(1.0, 1.0, 1.0);
    glUniform3fv(uniform_id, 1, &white[0]);

    glDrawArrays(GL_LINES, 0, offset);
}

void GraphRendererOpenGL::draw_labels() const
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

    // Place a tick at every unit up the y axis
    for (double i = start; i < end; i += tick_spacing_major.y)
    {
        auto tick_y_vpspace = m_window.vp_matrix() * (m_view_matrix * glm::dvec3(0.0f, i, 1.0f));
        glm::ivec2 point(m_gutter_size_px - m_tick_len_px, tick_y_vpspace.y);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.y) << i;
        _draw_label(ss.str(),
                    point,
                    18,
                    7,
                    LabelAlignment::Right,
                    LabelAlignmentVertical::Center,
                    glm::vec3(1.0, 1.0, 1.0));
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
        _draw_label(ss.str(),
                    point,
                    18,
                    7,
                    LabelAlignment::Center,
                    LabelAlignmentVertical::Top,
                    glm::vec3(1.0, 1.0, 1.0));
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

void GraphRendererOpenGL::_draw_label(const std::string_view text,
                                      const glm::ivec2 &pos,
                                      int height,
                                      int width,
                                      LabelAlignment align,
                                      LabelAlignmentVertical valign,
                                      const glm::vec3 &colour) const
{
    glm::ivec2 offset = pos;
    glm::ivec2 char_stride = glm::ivec2(width, 0);

    if (align == LabelAlignment::Right)
    {
        offset -= char_stride * static_cast<int>(text.size());
    }
    else if (align == LabelAlignment::Center)
    {
        offset -= char_stride * static_cast<int>(text.size()) / 2;
    }

    if (valign == LabelAlignmentVertical::Center)
    {
        offset -= glm::ivec2(0, height / 2);
    }
    else if (valign == LabelAlignmentVertical::Bottom)
    {
        offset -= glm::ivec2(0, height);
    }

    GlyphData buffer[128];
    GlyphData *bufptr = &buffer[0];

    // offset.x = roundf(offset.x);
    // offset.y = roundf(offset.y);

    for (auto character : text)
    {
        _draw_glyph(character, offset, height, width, &bufptr);
        offset += char_stride;
    }

    std::size_t count = (bufptr - buffer);

    glBindVertexArray(_glyphbuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _glyphbuf_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GlyphData) * count, buffer);

    _glyph_shader.use();
    int uniform_id = _glyph_shader.uniform_location("view_matrix");
    const auto vp_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = _glyph_shader.uniform_location("glyph_colour");
    glUniform3fv(uniform_id, 1, &colour[0]);

    glBindTexture(GL_TEXTURE_2D, _glyph_texture);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glyphbuf_ebo);
    glDrawElements(GL_TRIANGLES, 6 * count, GL_UNSIGNED_INT, 0);
}

void GraphRendererOpenGL::_draw_glyph(
    char character, const glm::ivec2 &pos, int height, int width, GlyphData **buf) const
{
    GlyphData *data = *buf;
    data->verts[0].vert = pos;
    data->verts[1].vert = pos + glm::ivec2(width, 0);
    data->verts[2].vert = pos + glm::ivec2(0, height);
    data->verts[3].vert = pos + glm::ivec2(width, height);

    // Look up the texture coordinate for the character
    const int COLS = 16;
    const int ROWS = 8;
    int col = character % COLS;
    int row = character / COLS;
    const float COL_STRIDE = 1.0f / COLS;
    const float GLYPH_WIDTH = (static_cast<float>(width) / static_cast<float>(height)) / COLS;
    const float ROW_STRIDE = 1.0f / ROWS;
    const float GLYPH_HEIGHT = 1.0f / ROWS;

    data->verts[0].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row);
    data->verts[1].tex_coords = glm::vec2(COL_STRIDE * col + GLYPH_WIDTH, ROW_STRIDE * row);
    data->verts[2].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row + GLYPH_HEIGHT);
    data->verts[3].tex_coords =
        glm::vec2(COL_STRIDE * col + GLYPH_WIDTH, ROW_STRIDE * row + GLYPH_HEIGHT);

    *buf = data + 1;
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
