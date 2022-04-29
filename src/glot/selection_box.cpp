#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "selection_box.hpp"
#include "resources.hpp"

SelectionBox::SelectionBox(Window &window) : m_window(window), m_colour(1.0, 1.0, 1.0)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(BoxVerticies), nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("block/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("block/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_shader = Program(shaders);
}

SelectionBox::~SelectionBox()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void SelectionBox::set_position(const glm::ivec2 &position)
{
    m_position = position;
}

void SelectionBox::set_size(const glm::ivec2 &size)
{
    m_size = size;
}

void SelectionBox::draw() const
{
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    m_shader.use();
    int uniform_id = m_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(m_window.vp_matrix_inv()[0]));

    uniform_id = m_shader.uniform_location("colour");
    glUniform3fv(uniform_id, 1, &m_colour[0]);

    uniform_id = m_shader.uniform_location("alpha");
    glUniform1f(uniform_id, 0.5);

    auto align_to_pixel = [](const glm::ivec2 &in) { return glm::vec2(round(in.x), round(in.y)); };

    BoxVerticies verts{align_to_pixel(m_position),
                       align_to_pixel(glm::vec2(m_position.x + m_size.x, m_position.y)),
                       align_to_pixel(glm::vec2(m_position.x, m_position.y + m_size.y)),
                       align_to_pixel(m_position + m_size)};

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(BoxVerticies), &verts);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
