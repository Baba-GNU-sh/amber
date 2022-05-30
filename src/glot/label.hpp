#pragma once

#include <string>
#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "font.hpp"
#include "view.hpp"

class Label : public View
{
  public:
    enum class AlignmentHorizontal
    {
        Left,
        Right,
        Center
    };

    enum class AlignmentVertical
    {
        Top,
        Center,
        Bottom
    };

    Label(Font &material, int capacity = 128);
    Label(Font &material, const std::string &text, int capacity = 128);
    ~Label();

    Label(const Label &) = delete;
    Label &operator=(const Label &) = delete;
    Label(Label &&);
    Label &operator=(Label &&) = delete;

    void set_text(const std::string &string);
    void set_colour(const glm::vec3 &colour);
    void set_position(const glm::dvec2 &position) override;
    glm::dvec2 position() const override;
    void set_alignment(AlignmentHorizontal halign);
    void set_alignment(AlignmentVertical valign);
    void draw(const Window &window) override;
    glm::dvec2 size() const override;

  private:
    struct GlyphVertex
    {
        glm::vec2 vert;
        glm::vec2 tex_coords;
    };

    struct GlyphVerticies
    {
        GlyphVertex verts[4];
    };

    void initialize_buffers();
    void draw_glyph(char character, const glm::ivec2 &pos, GlyphVerticies **buf) const;

    Font &m_material;
    std::size_t m_capacity;
    std::string m_text;
    glm::vec3 m_colour;
    glm::dvec2 m_position;
    AlignmentHorizontal m_halign;
    AlignmentVertical m_valign;

    static constexpr int GLYPH_HEIGHT = 18;
    static constexpr int GLYPH_WIDTH = 7;

    unsigned int m_glyphbuf_vao;
    unsigned int m_glyphbuf_vbo;
    unsigned int m_glyphbuf_ebo;
};
