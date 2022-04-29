#pragma once

#include <bits/c++config.h>
#include <string>
#include "window.hpp"
#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "font_material.hpp"

class Label
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

    Label(Window &window, FontMaterial &material, int capacity = 128);
    ~Label();

    Label(const Label &) = delete;
    Label &operator=(const Label &) = delete;
    Label(Label &&);
    Label &operator=(Label &&) = delete;

    void set_text(const std::string &string);
    void set_colour(const glm::vec3 &colour);
    void set_position(const glm::ivec2 &position);
    void set_alignment(LabelAlignmentHorizontal halign);
    void set_alignment(LabelAlignmentVertical valign);

    void draw_text(LabelAlignmentHorizontal halign, LabelAlignmentVertical valign) const;

  private:
    void draw_glyph(char character, const glm::ivec2 &pos, GlyphData **buf) const;

    Window &m_window;
    FontMaterial &m_material;
    std::size_t m_capacity;
    std::string m_text;
    glm::vec3 m_colour;
    glm::ivec2 m_position;

    static constexpr int GLYPH_HEIGHT = 18;
    static constexpr int GLYPH_WIDTH = 7;

    unsigned int m_glyphbuf_vao;
    unsigned int m_glyphbuf_vbo;
    unsigned int m_glyphbuf_ebo;
};
