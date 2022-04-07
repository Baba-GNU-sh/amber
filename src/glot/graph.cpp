#include "graph.hpp"
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "resources.hpp"

GraphView::GraphView() : _position(0, 0), _size(100, 100), _dragging(false), _plot(_view_matrix)
{
    _update_view_matrix(glm::mat3(1.0f));

    // The idea is that we need to draw some lines, and some text.
    // The axes are just long lines, with smaller lines indicating the
    // ticks. There are some text labels on each of the ticks, and labels on
    // the axes indicating the units / scaling etc. Text is rendered using
    // bitmap fonts, and is made up of glyphs rendered using one quad per
    // glyph. When render is called, the verticies are updated, transferred
    // to the GPU, then two draw calls are made, one to render the lines,
    // and one to render the glyphs.

    _init_line_buffers();
    _init_glyph_buffers();
}

GraphView::~GraphView()
{
}

bool GraphView::_hittest(glm::vec2 value, glm::vec2 tl, glm::vec2 br)
{
    if (value.x < tl.x)
        return false;
    if (value.x > br.x)
        return false;
    if (value.y < tl.y)
        return false;
    if (value.y > br.y)
        return false;
    return true;
}

void GraphView::_init_line_buffers()
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

void GraphView::_init_glyph_buffers()
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

void GraphView::draw(const glm::mat3 &viewport_matrix,
                     int plot_width,
                     const std::vector<TimeSeriesContainer> &time_series,
                     bool show_line_segments) const
{
    for (const auto &ts : time_series)
    {
        if (ts.visible)
        {
            _plot.draw(*(ts.ts), viewport_matrix, plot_width, ts.colour, show_line_segments);
        }
    }

    _draw_lines(viewport_matrix);
    _draw_labels(viewport_matrix);
}

void GraphView::_draw_lines(const glm::mat3 &vp_matrix) const
{
    int offset = 0;

    const auto tick_spacing = _tick_spacing(vp_matrix);
    const auto tick_spacing_major = std::get<0>(tick_spacing);
    const auto tick_spacing_minor = std::get<1>(tick_spacing);

    glBindVertexArray(_linebuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);

    // Get a pointer to the underlying buffer
    void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

    glm::vec2 tl(GUTTER_SIZE_PX, 0);
    glm::vec2 bl(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX);
    glm::vec2 br(_size.x, _size.y - GUTTER_SIZE_PX);

    // Draw the y axis line
    ptr[offset++] = tl;
    ptr[offset++] = bl;

    // Draw the x axis line
    ptr[offset++] = bl;
    ptr[offset++] = br;

    auto draw_ticks = [&](const glm::vec2 &tick_spacing,
                          const glm::vec2 &tick_size_y,
                          const glm::vec2 &tick_size_x) {
        // Draw the y-axis ticks
        // Work out where (in graph space) margin and height-margin is
        auto top_gs = screen2graph(vp_matrix, tl);
        auto bottom_gs = screen2graph(vp_matrix, bl);
        float start = ceilf(bottom_gs.y / tick_spacing.y) * tick_spacing.y;
        float end = ceilf(top_gs.y / tick_spacing.y) * tick_spacing.y;

        // Place a tick at every unit up the y axis
        for (float i = start; i < end; i += tick_spacing.y)
        {
            auto tick_y_vpspace = vp_matrix * (_view_matrix * glm::vec3(0.0f, i, 1.0f));
            ptr[offset++] = glm::vec2(GUTTER_SIZE_PX, tick_y_vpspace.y) + tick_size_y;
            ptr[offset++] = glm::vec2(GUTTER_SIZE_PX, tick_y_vpspace.y);
        }

        // Draw the y axis ticks
        auto left_gs = screen2graph(vp_matrix, bl);
        auto right_gs = screen2graph(vp_matrix, br);
        start = ceilf(left_gs.x / tick_spacing.x) * tick_spacing.x;
        end = ceilf(right_gs.x / tick_spacing.x) * tick_spacing.x;

        // Place a tick at every unit along the x axis
        for (float i = start; i < end; i += tick_spacing.x)
        {
            auto tick_x_vpspace = vp_matrix * (_view_matrix * glm::vec3(i, 0.0f, 1.0f));
            ptr[offset++] = glm::vec2(tick_x_vpspace.x, _size.y - GUTTER_SIZE_PX) + tick_size_x;
            ptr[offset++] = glm::vec2(tick_x_vpspace.x, _size.y - GUTTER_SIZE_PX);
        }
    };

    draw_ticks(tick_spacing_major, glm::vec2(-TICKLEN, 0), glm::vec2(0, TICKLEN));
    draw_ticks(tick_spacing_minor, glm::vec2(-TICKLEN / 2, 0), glm::vec2(0, TICKLEN / 2));

    // Add one additional vertical line where the cursor is
    ptr[offset++] = glm::vec2(_cursor.x, 0.0);
    ptr[offset++] = glm::vec2(_cursor.x, _size.y);

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
    const auto viewport_matrix_inv = glm::inverse(vp_matrix); // TODO this is expensive
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(viewport_matrix_inv[0]));
    glDrawArrays(GL_LINES, 0, offset);
}

void GraphView::_draw_labels(const glm::mat3 &vp_matrix) const
{
    glm::vec2 tl(GUTTER_SIZE_PX, 0);
    glm::vec2 bl(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX);
    glm::vec2 br(_size.x, _size.y - GUTTER_SIZE_PX);
    auto tick_spacing = _tick_spacing(vp_matrix);
    auto tick_spacing_major = std::get<0>(tick_spacing);
    // auto tick_spacing_minor = std::get<1>(tick_spacing);
    auto precision = std::get<2>(tick_spacing);

    // Draw one label per tick on the y axis
    auto top_gs = screen2graph(vp_matrix, tl);
    auto bottom_gs = screen2graph(vp_matrix, bl);
    float start = ceilf(bottom_gs.y / tick_spacing_major.y) * tick_spacing_major.y;
    float end = ceilf(top_gs.y / tick_spacing_major.y) * tick_spacing_major.y;

    // Place a tick at every unit up the y axis
    for (float i = start; i < end; i += tick_spacing_major.y)
    {
        auto tick_y_vpspace = vp_matrix * (_view_matrix * glm::vec3(0.0f, i, 1.0f));
        glm::vec2 point(GUTTER_SIZE_PX - TICKLEN, tick_y_vpspace.y);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.y) << i;
        _draw_label(vp_matrix,
                    ss.str(),
                    point,
                    18,
                    7,
                    LabelAlignment::Right,
                    LabelAlignmentVertical::Center);
    }

    // Draw the y axis ticks
    auto left_gs = screen2graph(vp_matrix, bl);
    auto right_gs = screen2graph(vp_matrix, br);
    start = ceilf(left_gs.x / tick_spacing_major.x) * tick_spacing_major.x;
    end = ceilf(right_gs.x / tick_spacing_major.x) * tick_spacing_major.x;

    // Place a tick at every unit along the x axis
    for (float i = start; i < end; i += tick_spacing_major.x)
    {
        auto tick_x_vpspace = vp_matrix * (_view_matrix * glm::vec3(i, 0.0f, 1.0f));
        glm::vec2 point(tick_x_vpspace.x, _size.y - GUTTER_SIZE_PX + TICKLEN);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision.x) << i;
        _draw_label(
            vp_matrix, ss.str(), point, 18, 7, LabelAlignment::Center, LabelAlignmentVertical::Top);
    }

    // Find the nearest sample and draw labels for it
    // auto cursor_gs = glm::inverse(_view_matrix) * _viewport_matrix_inv *
    // glm::vec3(_cursor, 1.0f); auto cursor_gs2 = glm::inverse(_view_matrix) * _viewport_matrix_inv
    // *
    //                   glm::vec3(_cursor.x + 1.0f, _cursor.y, 1.0f);
    // auto sample = _ts->get_sample(cursor_gs.x, cursor_gs2.x - cursor_gs.x);

    // const auto draw_label = [&](double value) {
    //     glm::vec2 sample_gs(cursor_gs.x, value);

    //     glm::vec3 point3 = _viewport_matrix * _view_matrix * glm::vec3(sample_gs, 1.0f);
    //     glm::vec2 point(point3.x, point3.y);

    //     std::stringstream ss;
    //     ss << value;
    //     _draw_label(ss.str(), point, 18, 7, LabelAlignment::Right,
    //     LabelAlignmentVertical::Center);
    // };

    // draw_label(sample.average);
    // // draw_label(sample.min);
    // // draw_label(sample.max);
}

void GraphView::_draw_label(const glm::mat3 &vp_matrix,
                            const std::string_view text,
                            const glm::vec2 &pos,
                            float height,
                            float width,
                            LabelAlignment align,
                            LabelAlignmentVertical valign) const
{
    glm::vec2 offset = pos;
    glm::vec2 delta = glm::vec2(width, 0);

    if (align == LabelAlignment::Right)
    {
        offset -= delta * static_cast<float>(text.size());
    }
    else if (align == LabelAlignment::Center)
    {
        offset -= delta * static_cast<float>(text.size()) / 2.0f;
    }

    if (valign == LabelAlignmentVertical::Center)
    {
        offset -= glm::vec2(0, width);
    }
    else if (valign == LabelAlignmentVertical::Bottom)
    {
        offset -= glm::vec2(0, height);
    }

    GlyphData buffer[128];
    GlyphData *bufptr = &buffer[0];

    offset.x = roundf(offset.x);
    offset.y = roundf(offset.y);

    for (auto c : text)
    {
        _draw_glyph(c, offset, height, width, &bufptr);
        offset += delta;
    }

    std::size_t count = (bufptr - buffer);

    glBindVertexArray(_glyphbuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _glyphbuf_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GlyphData) * count, buffer);

    _glyph_shader.use();
    int uniform_id = _glyph_shader.uniform_location("view_matrix");
    const auto vp_matrix_inv = glm::inverse(vp_matrix);
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glyphbuf_ebo);
    glDrawElements(GL_TRIANGLES, 6 * count, GL_UNSIGNED_INT, 0);
    // glDrawArrays(GL_TRIANGLES, 0, 4 * (bufptr - buffer));
}

void GraphView::_draw_glyph(
    char c, const glm::vec2 &pos, float height, float width, GlyphData **buf) const
{
    GlyphData *data = *buf;
    data->verts[0].vert = pos;
    data->verts[1].vert = pos + glm::vec2(width, 0.0f);
    data->verts[2].vert = pos + glm::vec2(0.0f, height);
    data->verts[3].vert = pos + glm::vec2(width, height);

    // Look up the texture coordinate for the character
    const int COLS = 16;
    const int ROWS = 8;
    int col = c % COLS;
    int row = c / COLS;
    const float COL_STRIDE = 1.0f / COLS;
    const float GLYPH_WIDTH = (width / height) / COLS;
    const float ROW_STRIDE = 1.0f / ROWS;
    const float GLYPH_HEIGHT = 1.0f / ROWS;

    data->verts[0].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row);
    data->verts[1].tex_coords = glm::vec2(COL_STRIDE * col + GLYPH_WIDTH, ROW_STRIDE * row);
    data->verts[2].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row + GLYPH_HEIGHT);
    data->verts[3].tex_coords =
        glm::vec2(COL_STRIDE * col + GLYPH_WIDTH, ROW_STRIDE * row + GLYPH_HEIGHT);

    *buf = data + 1;
}

std::tuple<glm::vec2, glm::vec2, glm::ivec2> GraphView::_tick_spacing(
    const glm::mat3 &vp_matrix) const
{
    const glm::vec2 MIN_TICK_SPACING_PX(80, 50);

    // Calc the size of this vector in graph space (ignoring translation & sign)
    const glm::vec2 min_tick_spacing_gs = glm::abs(screen2graph(vp_matrix, glm::vec2(0.0f, 0.0f)) -
                                                   screen2graph(vp_matrix, MIN_TICK_SPACING_PX));

    // Round this size up to the nearest power of 10
    glm::vec2 tick_spacing;
    tick_spacing.x = powf(10.0f, ceilf(log10f(min_tick_spacing_gs.x)));
    tick_spacing.y = powf(10.0f, ceilf(log10f(min_tick_spacing_gs.y)));
    glm::vec2 minor_tick_spacing = tick_spacing / 2.0f;

    glm::ivec2 precision;
    precision.x = -ceilf(log10f(min_tick_spacing_gs.x));
    precision.y = -ceilf(log10f(min_tick_spacing_gs.y));

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

void GraphView::set_size(int width, int height)
{
    _size = glm::vec2(width, height);
    _plot.set_size(width, height);
}

glm::ivec2 GraphView::size() const
{
    return _size;
}

void GraphView::set_position(int x, int y)
{
    _position = glm::vec2(x, y);
}

glm::ivec2 GraphView::position() const
{
    return _position;
}

void GraphView::mouse_button(int button, int action, int mods)
{
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        _dragging = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        _dragging = false;
    }
}

void GraphView::cursor_move(const glm::mat3 &vp_matrix, double xpos, double ypos)
{
    glm::ivec2 new_cursor(xpos, ypos);

    if (_dragging)
    {
        auto cursor_gs_old = screen2graph(vp_matrix, _cursor);
        auto cursor_gs_new = screen2graph(vp_matrix, new_cursor);
        auto cursor_gs_delta = cursor_gs_new - cursor_gs_old;
        _update_view_matrix(glm::translate(_view_matrix, cursor_gs_delta));
    }

    _cursor = new_cursor;
}

void GraphView::mouse_scroll(const glm::mat3 &vp_matrix, double /*xoffset*/, double yoffset)
{
    const float zoom_delta = 1.0f + (yoffset / 10.0f);
    glm::vec2 zoom_delta_vec(1.0);

    // Is the cursor in the gutter?
    if (_hittest(_cursor, glm::vec2(0, 0), glm::vec2(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX)))
    {
        // We are in the y gutter
        zoom_delta_vec.y = zoom_delta;
    }
    else if (_hittest(_cursor,
                      glm::vec2(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX),
                      glm::vec2(_size.x, _size.y)))
    {
        zoom_delta_vec.x = zoom_delta;
    }
    else if (_hittest(_cursor,
                      glm::vec2(GUTTER_SIZE_PX, 0),
                      glm::vec2(_size.x, _size.y - GUTTER_SIZE_PX)))
    {
        zoom_delta_vec = glm::vec2(zoom_delta);
    }

    // Work out where the pointer is in graph space
    auto cursor_in_gs_old = screen2graph(vp_matrix, _cursor);

    _update_view_matrix(glm::scale(_view_matrix, zoom_delta_vec));

    auto cursor_in_gs_new = screen2graph(vp_matrix, _cursor);
    auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;

    _update_view_matrix(glm::translate(_view_matrix, cursor_delta));
}

glm::mat3 GraphView::view_matrix() const
{
    return _view_matrix;
}

glm::vec2 GraphView::cursor_graphspace(const glm::mat3 &vp_matrix) const
{
    return screen2graph(vp_matrix, _cursor);
}

glm::vec2 GraphView::screen2graph(const glm::mat3 &viewport_matrix, const glm::ivec2 &value) const
{
    const glm::vec3 value3(value, 1.0f);
    const auto viewport_matrix_inv = glm::inverse(viewport_matrix);
    auto value_cs = viewport_matrix_inv * value3;
    auto value_gs = _view_matrix_inv * value_cs;
    return value_gs;
}

void GraphView::_update_view_matrix(const glm::mat3 &value)
{
    _view_matrix = value;
    _view_matrix_inv = glm::inverse(value);
}
