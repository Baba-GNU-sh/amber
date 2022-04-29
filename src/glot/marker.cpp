#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <stb_image/stb_image.h>

#include "marker.hpp"
#include "label.hpp"
#include <database/timeseries.hpp>
#include "resources.hpp"

struct TextureCoord
{
    glm::vec2 vertex_pos;
    glm::vec2 texture_pos;
};

Marker::Marker(Window &window)
    : m_window(window), m_handle(window, "marker_center.png")
{
    glGenBuffers(1, &m_line_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_line_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::ivec2), nullptr, GL_STREAM_DRAW);

    glGenVertexArrays(1, &m_line_vao);
    glBindVertexArray(m_line_vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
    glEnableVertexAttribArray(0);

    std::vector<Shader> line_shaders{
        Shader(Resources::find_shader("block/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("block/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_line_shader = Program(line_shaders);

    m_handle.set_alignment(Sprite::HorizontalAlignment::Center);
    m_handle.set_alignment(Sprite::VerticalAlignment::Top);
}

Marker::~Marker()
{
    glDeleteBuffers(1, &m_line_vertex_buffer);
    glDeleteVertexArrays(1, &m_line_vao);
}

void Marker::set_position(const glm::ivec2 &position)
{
    m_position = position;
    m_handle.set_position(m_position + glm::ivec2(0, m_height));
}

void Marker::set_colour(const glm::vec3 &colour)
{
    m_colour = colour;
    m_handle.set_tint(colour);
}

void Marker::set_height(int height)
{
    m_height = height;
    m_handle.set_position(m_position + glm::ivec2(0, m_height));
}

void Marker::draw() const
{
    m_handle.draw();

    // Align to the nearest half-pixel
    glm::vec2 position_float = m_position;
    position_float.x = round(position_float.x + 0.5) - 0.5;

    // Draw the vertical line
    const auto vp_matrix_inv = m_window.vp_matrix_inv();
    m_line_shader.use();
    int uniform_id = m_line_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_line_shader.uniform_location("colour");
    glUniform3fv(uniform_id, 1, &m_colour[0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_line_vertex_buffer);

    glm::vec2 line_verticies[2];
    line_verticies[0] = position_float;
    line_verticies[1] = position_float + glm::vec2(0, m_height);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_verticies), line_verticies);

    glBindVertexArray(m_line_vao);
    glDrawArrays(GL_LINES, 0, 2);
}

unsigned int Marker::load_texture(const std::string &file_name) const
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

    return texture_handle;
}
