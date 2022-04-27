#pragma once

#include <string>
#include "window.hpp"
#include <glm/glm.hpp>
#include "shader_utils.hpp"

class TextRendererOpenGL
{
  public:
    enum class LabelAlignmentHorizontal
    {
        Left,
        Right,
        Center
    };

    enum class LabelAlignmentVertical
    {
        Top,
        Center,
        Bottom
    };

    struct GlyphVertex
    {
        glm::vec2 vert;
        glm::vec2 tex_coords;
    };

    struct GlyphData
    {
        GlyphVertex verts[4]; // Order: [TL, TR, BL, BR]
    };

    TextRendererOpenGL(Window &window, const std::string &font_filename);
    ~TextRendererOpenGL();

    void draw_text(const std::string &text,
                   const glm::ivec2 &pos,
                   LabelAlignmentHorizontal halign,
                   LabelAlignmentVertical valign,
                   const glm::vec3 &colour) const;

  private:
    void draw_glyph(char character, const glm::ivec2 &pos, GlyphData **buf) const;

    Window &m_window;

    static constexpr int GLYPH_HEIGHT = 18;
    static constexpr int GLYPH_WIDTH = 7;

    unsigned int m_font_atlas_tex;
    unsigned int m_glyphbuf_vao;
    unsigned int m_glyphbuf_vbo;
    unsigned int m_glyphbuf_ebo;
    Program m_glyph_shader;
};
