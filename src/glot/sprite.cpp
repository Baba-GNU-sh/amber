#include "sprite.hpp"
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <stb_image/stb_image.h>
#include <vector>
#include "resources.hpp"

Sprite::Sprite(const Window &window, const std::string &file_name) : m_window(window)
{
    std::tie(m_texture, m_size) = load_texture(file_name);

    // Allocate enough buffer space for 4 verts and 4 texture coords
    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextureCoord) * 4, nullptr, GL_STREAM_DRAW);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextureCoord), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(TextureCoord),
                          reinterpret_cast<void *>(offsetof(TextureCoord, texture_pos)));
    glEnableVertexAttribArray(1);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("sprite/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("sprite/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_shader = Program(shaders);
}

Sprite::~Sprite()
{
    glDeleteTextures(1, &m_texture);
    glDeleteBuffers(1, &m_vertex_buffer);
    glDeleteVertexArrays(1, &m_vao);
}

void Sprite::set_position(const glm::ivec2 &position)
{
    m_position = position;
}

void Sprite::set_alignment(AlignmentVertical align)
{
    m_vertical_alignment = align;
}

void Sprite::set_alignment(AlignmentHorizontal align)
{
    m_horizontal_alignment = align;
}

void Sprite::set_tint(const glm::vec3 &colour)
{
    m_tint_colour = colour;
}

void Sprite::draw() const
{
    m_shader.use();
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    glBindVertexArray(m_vao);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    int uniform_id = m_shader.uniform_location("view_matrix");
    const auto vp_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_shader.uniform_location("tint_colour");
    glUniform3fv(uniform_id, 1, &m_tint_colour[0]);

    auto position = m_position;
    switch (m_horizontal_alignment)
    {
    case AlignmentHorizontal::Center:
        position.x -= m_size.x / 2;
        break;
    case AlignmentHorizontal::Right:
        position.x -= m_size.x;
        break;
    default:
        break;
    }

    // TODO use alignment rules
    TextureCoord data[4];
    data[0].vertex_pos = position;
    data[0].texture_pos = glm::vec2(0.0, 0.0);
    data[1].vertex_pos = glm::vec2(position.x, position.y + m_size.y);
    data[1].texture_pos = glm::vec2(0.0, 1.0);
    data[2].vertex_pos = glm::vec2(position.x + m_size.x, position.y);
    data[2].texture_pos = glm::vec2(1.0, 0.0);
    data[3].vertex_pos = position + m_size;
    data[3].texture_pos = glm::vec2(1.0, 1.0);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

std::pair<unsigned int, glm::ivec2> Sprite::load_texture(const std::string &file_name)
{
    unsigned int texture_handle;
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    auto file_path = Resources::find_sprite(file_name);
    unsigned char *tex_data = stbi_load(file_path.c_str(), &width, &height, &nrChannels, 0);
    if (!tex_data)
    {
        throw std::runtime_error("Unable to load marker texture: " +
                                 std::string(stbi_failure_reason()));
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    stbi_image_free(tex_data);

    return std::make_pair(texture_handle, glm::ivec2(width, height));
}
