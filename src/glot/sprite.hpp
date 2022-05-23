#pragma once

#include <string>
#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "window.hpp"
#include "view.hpp"

class Sprite : public View
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

    Sprite(const std::string &filename);
    ~Sprite();
    Sprite(const Sprite &) = delete;
    Sprite &operator=(const Sprite &) = delete;
    Sprite(Sprite &&) = delete;
    Sprite &operator=(Sprite &&) = delete;

    void set_position(const glm::dvec2 &pos) override;
    glm::dvec2 position() const override;
    glm::dvec2 size() const override;

    void set_alignment(AlignmentVertical align);
    void set_alignment(AlignmentHorizontal align);
    void set_tint(const glm::vec3 &colour);
    virtual void draw(const Window &window) const override;

  private:
    struct TextureCoord
    {
        glm::vec2 vertex_pos;
        glm::vec2 texture_pos;
    };

    static std::pair<unsigned int, glm::ivec2> load_texture(const std::string &file_name);

    unsigned int m_vertex_buffer;
    unsigned int m_vao;
    unsigned int m_texture;
    Program m_shader;
    glm::dvec2 m_position = glm::dvec2(0.0);
    glm::dvec2 m_size = glm::dvec2(0.0);
    AlignmentVertical m_vertical_alignment = AlignmentVertical::Top;
    AlignmentHorizontal m_horizontal_alignment = AlignmentHorizontal::Left;
    glm::vec3 m_tint_colour = glm::vec3(1.0);
};
