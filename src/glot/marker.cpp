#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <stb_image/stb_image.h>
#include <sstream>

#include "marker.hpp"
#include "label.hpp"
#include <database/timeseries.hpp>
#include "resources.hpp"

Marker::Marker(Window &window)
    : m_window(window), m_handle(window, "marker_center.png"), m_font("proggy_clean.png"),
      m_label(window, m_font)
{
    glGenBuffers(1, &m_line_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_line_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), nullptr, GL_STREAM_DRAW);

    glGenVertexArrays(1, &m_line_vao);
    glBindVertexArray(m_line_vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
    glEnableVertexAttribArray(0);

    std::vector<Shader> line_shaders{
        Shader(Resources::find_shader("block/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("block/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_line_shader = Program(line_shaders);

    m_handle.set_alignment(Sprite::AlignmentHorizontal::Center);
    m_handle.set_alignment(Sprite::AlignmentVertical::Top);
    m_label.set_alignment(Label::AlignmentHorizontal::Center);
    m_label.set_alignment(Label::AlignmentVertical::Top);
}

Marker::~Marker()
{
    glDeleteBuffers(1, &m_line_vertex_buffer);
    glDeleteVertexArrays(1, &m_line_vao);
}

double Marker::x_position() const
{
    return m_position;
}

void Marker::set_x_position(double position)
{
    m_position = position;
    update_layout();
}

void Marker::set_graph_transform(const Transform<double> &transform)
{
    m_graph_transform = transform;
    update_layout();
}

void Marker::set_screen_height(int height)
{
    m_height = height;
    update_layout();
}

void Marker::set_colour(const glm::vec3 &colour)
{
    m_colour = colour;
    m_handle.set_tint(colour);
    m_label.set_colour(colour);
}

void Marker::set_label_text(const std::string &text)
{
    m_label.set_text(text);
}

void Marker::set_visible(bool visible)
{
    m_is_visible = visible;
}

bool Marker::is_visible() const
{
    return m_is_visible;
}

void Marker::draw()
{
    if (!m_is_visible)
        return;

    m_handle.draw();
    m_label.draw();

    glm::dvec2 graph_pos(m_position, 0.0);
    auto position_ss = m_window.viewport_transform().apply(m_graph_transform.apply(graph_pos));
    position_ss.y = 0;
    position_ss.x = round(position_ss.x + 0.5) - 0.5;

    // Draw the vertical line
    const auto vp_matrix_inv = glm::mat3(m_window.viewport_transform().matrix_inverse());
    m_line_shader.use();
    int uniform_id = m_line_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(vp_matrix_inv[0]));

    uniform_id = m_line_shader.uniform_location("colour");
    glUniform3fv(uniform_id, 1, &m_colour[0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_line_vertex_buffer);

    glm::vec2 line_verticies[2];
    line_verticies[0] = position_ss;
    line_verticies[1] = position_ss + glm::dvec2(0, m_height);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_verticies), line_verticies);

    glBindVertexArray(m_line_vao);
    glDrawArrays(GL_LINES, 0, 2);
}

HitBox Marker::get_hitbox() const
{
    glm::dvec2 tl(m_handle.position().x - m_handle.size().x / 2, m_handle.position().y);
    return HitBox{tl, tl + m_handle.size()};
}

glm::dvec2 Marker::position() const
{
    return m_handle.position();
}

glm::dvec2 Marker::size() const
{
    return m_handle.size();
}

void Marker::on_mouse_button(const glm::dvec2 &, int button, int action, int)
{
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (!m_is_visible)
            return;
        m_is_dragging = true;
    }
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
    {
        m_is_dragging = false;
    }
}

void Marker::on_cursor_move(double x, double y)
{
    glm::dvec2 cursor(x, y);

    if (m_is_dragging)
    {
        const auto delta = cursor - m_cursor_old;
        on_drag(delta.x);
    }

    m_cursor_old = glm::dvec2(x, y);
}

void Marker::update_layout()
{
    glm::dvec2 graph_pos(m_position, 0.0);
    auto position_ss = m_window.viewport_transform().apply(m_graph_transform.apply(graph_pos));
    position_ss.y = 0;

    m_handle.set_position(position_ss + glm::dvec2(0, m_height));
    m_label.set_position(position_ss + glm::dvec2(0, m_height + 30));

    std::stringstream ss;
    ss << m_position;
    m_label.set_text(ss.str());
}
