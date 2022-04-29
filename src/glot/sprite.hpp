#pragma once

#include <string>
#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "window.hpp"

class Sprite
{
  public:
    enum class AlignmentVertical
    {
        Top,
        Middle,
        Bottom
    };

    enum class AlignmentHorizontal
    {
        Left,
        Center,
        Right
    };

    Sprite(const Window &window, const std::string &filename);
    ~Sprite();
    void set_position(const glm::ivec2 &pos);
    void set_alignment(AlignmentVertical align);
    void set_alignment(AlignmentHorizontal align);
    void set_tint(const glm::vec3 &colour);
    void draw() const;

  private:
    struct TextureCoord
    {
        glm::vec2 vertex_pos;
        glm::vec2 texture_pos;
    };

    static std::pair<unsigned int, glm::ivec2> load_texture(const std::string &file_name);

    const Window& m_window;
    unsigned int m_vertex_buffer;
    unsigned int m_vao;
    unsigned int m_texture;
    Program m_shader;
    glm::ivec2 m_position;
    glm::ivec2 m_size;
    AlignmentVertical m_vertical_alignment;
    AlignmentHorizontal m_horizontal_alignment;
    glm::vec3 m_tint_colour;
};
