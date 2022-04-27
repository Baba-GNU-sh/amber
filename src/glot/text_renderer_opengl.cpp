#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "text_renderer_opengl.hpp"
#include "resources.hpp"
#include "stb_image.h"

TextRendererOpenGL::TextRendererOpenGL(Window &window, const std::string &font_atlas_filename)
    : m_window(window)
{
    // Load the font atlas into a texture
    int width, height, nrChannels;
    const auto font_atlas_filepath = Resources::find_font(font_atlas_filename);
    unsigned char *tex_data =
        stbi_load(font_atlas_filepath.c_str(), &width, &height, &nrChannels, 0);
    if (!tex_data)
    {
        throw std::runtime_error("Unable to load font map: " + std::string(stbi_failure_reason()));
    }

    spdlog::info("font atlas: {}", nrChannels);

    glGenTextures(1, &m_font_atlas_tex);
    glBindTexture(GL_TEXTURE_2D, m_font_atlas_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);

    stbi_image_free(tex_data);

    // Initialize the buffers
    glGenVertexArrays(1, &m_glyphbuf_vao);
    glBindVertexArray(m_glyphbuf_vao);

    glGenBuffers(1, &m_glyphbuf_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_glyphbuf_vbo);

    glGenBuffers(1, &m_glyphbuf_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glyphbuf_ebo);

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
        Shader(Resources::find_shader("sprite/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("sprite/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_glyph_shader = Program(shaders);
}

TextRendererOpenGL::~TextRendererOpenGL()
{
    glDeleteTextures(1, &m_font_atlas_tex);
    glDeleteVertexArrays(1, &m_glyphbuf_vao);
    glDeleteBuffers(1, &m_glyphbuf_vbo);
    glDeleteBuffers(1, &m_glyphbuf_ebo);
}

void TextRendererOpenGL::draw_text(const std::string &text,
                                   const glm::ivec2 &pos,
                                   LabelAlignmentHorizontal halign,
                                   LabelAlignmentVertical valign,
                                   const glm::vec3 &colour) const
{
    glm::ivec2 offset = pos;
    glm::ivec2 char_stride = glm::ivec2(GLYPH_WIDTH, 0);

    if (halign == TextRendererOpenGL::LabelAlignmentHorizontal::Right)
    {
        offset -= char_stride * static_cast<int>(text.size());
    }
    else if (halign == TextRendererOpenGL::LabelAlignmentHorizontal::Center)
    {
        offset -= char_stride * static_cast<int>(text.size()) / 2;
    }

    if (valign == TextRendererOpenGL::LabelAlignmentVertical::Center)
    {
        offset -= glm::ivec2(0, GLYPH_HEIGHT / 2);
    }
    else if (valign == TextRendererOpenGL::LabelAlignmentVertical::Bottom)
    {
        offset -= glm::ivec2(0, GLYPH_HEIGHT);
    }

    GlyphData buffer[128];
    GlyphData *bufptr = &buffer[0];

    for (auto character : text)
    {
        draw_glyph(character, offset, &bufptr);
        offset += char_stride;
    }

    std::size_t count = (bufptr - buffer);

    glBindVertexArray(m_glyphbuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_glyphbuf_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GlyphData) * count, buffer);

    m_glyph_shader.use();
    int uniform_id = m_glyph_shader.uniform_location("view_matrix");
    const auto vp_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_glyph_shader.uniform_location("tint_colour");
    glUniform3fv(uniform_id, 1, &colour[0]);

    uniform_id = m_glyph_shader.uniform_location("depth");
    glUniform1f(uniform_id, -0.2);

    glBindTexture(GL_TEXTURE_2D, m_font_atlas_tex);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glyphbuf_ebo);
    glDrawElements(GL_TRIANGLES, 6 * count, GL_UNSIGNED_INT, 0);
}

void TextRendererOpenGL::draw_glyph(char character, const glm::ivec2 &pos, GlyphData **buf) const
{
    GlyphData *data = *buf;
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
