
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "line_renderer_opengl.hpp"
#include "resources.hpp"

LineRendererOpenGL::LineRendererOpenGL(Window &window) : m_window(window)
{
    glGenVertexArrays(1, &m_linebuf_vao);
    glBindVertexArray(m_linebuf_vao);

    glGenBuffers(1, &m_line_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_line_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("block/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("block/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_line_shader = Program(shaders);
}

LineRendererOpenGL::~LineRendererOpenGL()
{
    glDeleteVertexArrays(1, &m_linebuf_vao);
    glDeleteBuffers(1, &m_line_vertex_buffer);
}

void LineRendererOpenGL::draw_line(const glm::dvec2 &start,
                                   const glm::dvec2 &end,
                                   const glm::vec3 &colour) const
{
    // Draw the vertical line
    m_line_shader.use();
    int uniform_id = m_line_shader.uniform_location("view_matrix");
    const auto vp_matrix_inv = m_window.vp_matrix_inv();
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_line_shader.uniform_location("colour");
    glUniform3fv(uniform_id, 1, &colour[0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_line_vertex_buffer);

    glm::vec2 line_verticies[2];
    line_verticies[0] = start;
    line_verticies[1] = end;
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_verticies), line_verticies);

    glBindVertexArray(m_linebuf_vao);
    glDrawArrays(GL_LINES, 0, 2);
}
