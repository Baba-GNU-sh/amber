#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "marker_renderer_opengl.hpp"
#include "timeseries.hpp"
#include "resources.hpp"
#include "stb_image.h"

struct TextureCoord
{
    glm::vec2 vertex_pos;
    glm::vec2 texture_pos;
};

MarkerRendererOpenGL::MarkerRendererOpenGL(Window &window) : m_window(window)
{
    m_handle_texture = load_texture("marker_center.png");

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
        Shader(Resources::find_shader("glyph/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("glyph/fragment.glsl"), GL_FRAGMENT_SHADER)};

    m_shader_program = Program(shaders);
}

MarkerRendererOpenGL::~MarkerRendererOpenGL()
{
    glDeleteTextures(1, &m_handle_texture);
    glDeleteBuffers(1, &m_handle_vertex_buffer);
    glDeleteVertexArrays(1, &m_handle_vao);
}

void MarkerRendererOpenGL::draw(const std::string &label,
                                int position_px,
                                int gutter_size_px,
                                const glm::vec3 &colour) const
{
    (void)label;
    (void)position_px;
    (void)gutter_size_px;

    m_shader_program.use();
    glBindBuffer(GL_ARRAY_BUFFER, m_handle_vertex_buffer);
    glBindVertexArray(m_handle_vao);
    glBindTexture(GL_TEXTURE_2D, m_handle_texture);

    int uniform_id = m_shader_program.uniform_location("view_matrix");
    const auto vp_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_shader_program.uniform_location("glyph_colour");
    glUniform3fv(uniform_id, 1, &colour[0]);

    uniform_id = m_shader_program.uniform_location("depth");
    glUniform1f(uniform_id, -0.5);

    TextureCoord data[4];

    const auto window_size = m_window.size();
    const auto xlevel = window_size.y - gutter_size_px;

    int dims = 16;
    data[0].vertex_pos = glm::vec2(position_px - dims / 2, xlevel);
    data[0].texture_pos = glm::vec2(0.0, 0.0);
    data[1].vertex_pos = glm::vec2(position_px - dims / 2, xlevel + dims);
    data[1].texture_pos = glm::vec2(0.0, 1.0);
    data[2].vertex_pos = glm::vec2(position_px + dims / 2, xlevel);
    data[2].texture_pos = glm::vec2(1.0, 0.0);
    data[3].vertex_pos = glm::vec2(position_px + dims / 2, xlevel + dims);
    data[3].texture_pos = glm::vec2(1.0, 1.0);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
    auto file_path = Resources::find_texture(file_name);
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
