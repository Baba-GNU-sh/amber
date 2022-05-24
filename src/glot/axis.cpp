#include "axis.hpp"
#include "resources.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

Axis::Axis()
{
    glGenVertexArrays(1, &m_linebuf_vao);
    glBindVertexArray(m_linebuf_vao);

    glGenBuffers(1, &m_linebuf_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_linebuf_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 1024, nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("block/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("block/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_lines_shader = Program(shaders);
}

Axis::~Axis()
{
    glDeleteBuffers(1, &m_linebuf_vao);
    glDeleteVertexArrays(1, &m_linebuf_vao);
}

void Axis::draw(const Window &window) const
{
    int offset = 0;

    glBindVertexArray(m_linebuf_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_linebuf_vbo);

    // Get a pointer to the underlying buffer
    void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

    // Draw the y axis line
    ptr[offset++] = m_position;
    ptr[offset++] = m_position + m_size;

    // make sure to tell OpenGL we're done with the pointer
    glUnmapBuffer(GL_ARRAY_BUFFER);

    m_lines_shader.use();
    int uniform_id = m_lines_shader.uniform_location("view_matrix");
    const auto viewport_matrix_inv = glm::mat3(window.viewport_transform().matrix_inverse());
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(viewport_matrix_inv[0]));

    uniform_id = m_lines_shader.uniform_location("colour");
    glm::vec3 white(1.0, 1.0, 1.0);
    glUniform3fv(uniform_id, 1, &white[0]);

    glDrawArrays(GL_LINES, 0, offset);
}

glm::dvec2 Axis::position() const
{
    return m_position;
}
void Axis::set_position(const glm::dvec2 &position)
{
    m_position = position;
}

glm::dvec2 Axis::size() const
{
    return m_size;
}
void Axis::set_size(const glm::dvec2 &size)
{
    m_size = size;
}

void Axis::set_orientation(Orientation ori)
{
    m_orientation = ori;
}

void Axis::on_scroll(double xoffset, double yoffset)
{
    spdlog::info("Scroll {} {}", xoffset, yoffset);
}
