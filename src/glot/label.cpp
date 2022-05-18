#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image/stb_image.h>
#include <stdexcept>

#include "label.hpp"
#include "resources.hpp"
#include "font.hpp"

Label::Label(Window &window, Font &material, int capacity)
    : m_window(window), m_material(material), m_capacity(capacity)
{
    // Initialize the buffers
    glGenVertexArrays(1, &m_glyphbuf_vao);
    glBindVertexArray(m_glyphbuf_vao);

    glGenBuffers(1, &m_glyphbuf_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_glyphbuf_vbo);

    glGenBuffers(1, &m_glyphbuf_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glyphbuf_ebo);

    const int sz = 6 * capacity / 4;
    std::vector<unsigned int> indicies(sz);
    for (int i = 0, j = 0; j < sz; i += 4, j += 6)
    {
        indicies[j] = i;
        indicies[j + 1] = i + 1;
        indicies[j + 2] = i + 2;
        indicies[j + 3] = i + 1;
        indicies[j + 4] = i + 2;
        indicies[j + 5] = i + 3;
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(unsigned int) * indicies.size(),
                 indicies.data(),
                 GL_STATIC_DRAW);

    // A glyph is rendered as a quad so we only need 4 verts and 4 texture
    // lookups
    glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(GlyphVerticies), nullptr, GL_STREAM_DRAW);

    // Define an attribute for the glyph verticies
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void *)0);
    glEnableVertexAttribArray(0);

    // Define an attribute for the texture lookups
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void *)offsetof(GlyphVertex, tex_coords));
    glEnableVertexAttribArray(1);
}

Label::Label(Label &&other)
    : m_window(other.m_window), m_material(other.m_material), m_capacity(other.m_capacity),
      m_colour(other.m_colour)
{
    m_glyphbuf_vao = other.m_glyphbuf_vao;
    other.m_glyphbuf_vao = 0;
    m_glyphbuf_vbo = other.m_glyphbuf_vbo;
    other.m_glyphbuf_vbo = 0;
    m_glyphbuf_ebo = other.m_glyphbuf_ebo;
    other.m_glyphbuf_ebo = 0;
}

Label::~Label()
{
    if (m_glyphbuf_vao)
        glDeleteVertexArrays(1, &m_glyphbuf_vao);
    if (m_glyphbuf_vbo)
        glDeleteBuffers(1, &m_glyphbuf_vbo);
    if (m_glyphbuf_ebo)
        glDeleteBuffers(1, &m_glyphbuf_ebo);
}

void Label::set_text(const std::string &text)
{
    if (text.length() > m_capacity)
    {
        throw std::runtime_error("String is longer than capacity!");
    }
    m_text = text;
}

void Label::set_colour(const glm::vec3 &colour)
{
    m_colour = colour;
}

void Label::set_position(const glm::ivec2 &position)
{
    m_position = position;
}

void Label::set_alignment(AlignmentHorizontal halign)
{
    m_halign = halign;
}

void Label::set_alignment(AlignmentVertical valign)
{
    m_valign = valign;
}

void Label::draw() const
{
    glm::ivec2 offset = m_position;
    glm::ivec2 char_stride = glm::ivec2(GLYPH_WIDTH, 0);

    if (m_halign == Label::AlignmentHorizontal::Right)
    {
        offset -= char_stride * static_cast<int>(m_text.size());
    }
    else if (m_halign == Label::AlignmentHorizontal::Center)
    {
        offset -= char_stride * static_cast<int>(m_text.size()) / 2;
    }

    if (m_valign == Label::AlignmentVertical::Center)
    {
        offset -= glm::ivec2(0, GLYPH_HEIGHT / 2);
    }
    else if (m_valign == Label::AlignmentVertical::Bottom)
    {
        offset -= glm::ivec2(0, GLYPH_HEIGHT);
    }

    GlyphVerticies buffer[128];
    GlyphVerticies *bufptr = &buffer[0];

    for (auto character : m_text)
    {
        draw_glyph(character, offset, &bufptr);
        offset += char_stride;
    }

    std::size_t count = (bufptr - buffer);

    glBindVertexArray(m_glyphbuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_glyphbuf_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GlyphVerticies) * count, buffer);

    const auto vp_matrix_inv = m_window.viewport_transform().matrix_inverse();
    m_material.use(m_colour, vp_matrix_inv);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glyphbuf_ebo);
    glDrawElements(GL_TRIANGLES, 6 * count, GL_UNSIGNED_INT, 0);
}

void Label::draw_glyph(char character, const glm::ivec2 &pos, GlyphVerticies **buf) const
{
    GlyphVerticies *data = *buf;
    data->verts[0].vert = pos;
    data->verts[1].vert = pos + glm::ivec2(GLYPH_WIDTH, 0);
    data->verts[2].vert = pos + glm::ivec2(0, GLYPH_HEIGHT);
    data->verts[3].vert = pos + glm::ivec2(GLYPH_WIDTH, GLYPH_HEIGHT);

    // Look up the texture coordinate for the character
    const int COLS = 16;
    const int ROWS = 8;
    int col = character % COLS;
    int row = character / COLS;
    const float COL_STRIDE = 1.0f / COLS;
    const float ATLAS_GLYPH_WIDTH =
        (static_cast<float>(GLYPH_WIDTH) / static_cast<float>(GLYPH_HEIGHT)) / COLS;
    const float ROW_STRIDE = 1.0f / ROWS;
    const float ATLAS_GLYPH_HEIGHT = 1.0f / ROWS;

    data->verts[0].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row);
    data->verts[1].tex_coords = glm::vec2(COL_STRIDE * col + ATLAS_GLYPH_WIDTH, ROW_STRIDE * row);
    data->verts[2].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row + ATLAS_GLYPH_HEIGHT);
    data->verts[3].tex_coords =
        glm::vec2(COL_STRIDE * col + ATLAS_GLYPH_WIDTH, ROW_STRIDE * row + ATLAS_GLYPH_HEIGHT);

    *buf = data + 1;
}
