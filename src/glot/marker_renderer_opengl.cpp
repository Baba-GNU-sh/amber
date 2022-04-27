#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <stb_image/stb_image.h>

#include "marker_renderer_opengl.hpp"
#include "text_renderer_opengl.hpp"
#include "timeseries.hpp"
#include "resources.hpp"

struct TextureCoord
{
    glm::vec2 vertex_pos;
    glm::vec2 texture_pos;
};

MarkerRendererOpenGL::MarkerRendererOpenGL(Window &window) : m_window(window)
{
    m_handle_texture_center = load_texture("marker_center.png");
    m_handle_texture_left = load_texture("marker_left.png");
    m_handle_texture_right = load_texture("marker_right.png");

    // Allocate enough buffer space for 4 verts and 4 texture coords
    glGenBuffers(1, &m_handle_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_handle_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextureCoord) * 4, nullptr, GL_STREAM_DRAW);

    glGenVertexArrays(1, &m_handle_vao);
    glBindVertexArray(m_handle_vao);

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
    m_sprite_shader = Program(shaders);

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
}

MarkerRendererOpenGL::~MarkerRendererOpenGL()
{
    glDeleteTextures(1, &m_handle_texture_center);
    glDeleteTextures(1, &m_handle_texture_left);
    glDeleteTextures(1, &m_handle_texture_right);
    glDeleteBuffers(1, &m_handle_vertex_buffer);
    glDeleteVertexArrays(1, &m_handle_vao);
    glDeleteBuffers(1, &m_line_vertex_buffer);
    glDeleteVertexArrays(1, &m_line_vao);
}

void MarkerRendererOpenGL::draw(int position_px,
                                int gutter_size_px,
                                const glm::vec3 &colour,
                                MarkerStyle style) const
{
    // Align to the nearest half-pixel
    const auto position_px_float = round(static_cast<double>(position_px) + 0.5) - 0.5;

    m_sprite_shader.use();
    glBindBuffer(GL_ARRAY_BUFFER, m_handle_vertex_buffer);
    glBindVertexArray(m_handle_vao);

    switch (style)
    {
    case MarkerStyle::Center:
        glBindTexture(GL_TEXTURE_2D, m_handle_texture_center);
        break;
    case MarkerStyle::Left:
        glBindTexture(GL_TEXTURE_2D, m_handle_texture_left);
        break;
    case MarkerStyle::Right:
        glBindTexture(GL_TEXTURE_2D, m_handle_texture_right);
        break;
    }

    int uniform_id = m_sprite_shader.uniform_location("view_matrix");
    const auto vp_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_sprite_shader.uniform_location("tint_colour");
    glUniform3fv(uniform_id, 1, &colour[0]);

    uniform_id = m_sprite_shader.uniform_location("depth");
    glUniform1f(uniform_id, -0.5);

    TextureCoord data[4];

    const auto window_size = m_window.size();
    const auto xlevel = window_size.y - gutter_size_px;

    int dims = 16;
    data[0].vertex_pos = glm::vec2(position_px_float - dims / 2, xlevel);
    data[0].texture_pos = glm::vec2(0.0, 0.0);
    data[1].vertex_pos = glm::vec2(position_px_float - dims / 2, xlevel + dims);
    data[1].texture_pos = glm::vec2(0.0, 1.0);
    data[2].vertex_pos = glm::vec2(position_px_float + dims / 2, xlevel);
    data[2].texture_pos = glm::vec2(1.0, 0.0);
    data[3].vertex_pos = glm::vec2(position_px_float + dims / 2, xlevel + dims);
    data[3].texture_pos = glm::vec2(1.0, 1.0);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Draw the vertical line
    m_line_shader.use();
    uniform_id = m_line_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_line_shader.uniform_location("colour");
    glUniform3fv(uniform_id, 1, &colour[0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_line_vertex_buffer);

    glm::vec2 line_verticies[2];
    line_verticies[0] = glm::vec2(position_px_float, 0.0);
    line_verticies[1] = glm::vec2(position_px_float, xlevel);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_verticies), line_verticies);

    glBindVertexArray(m_line_vao);
    glDrawArrays(GL_LINES, 0, 2);
}

unsigned int MarkerRendererOpenGL::load_texture(const std::string &file_name) const
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
