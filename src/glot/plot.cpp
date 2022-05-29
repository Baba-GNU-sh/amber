#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "plot.hpp"
#include <database/timeseries.hpp>
#include "resources.hpp"

Plot::Plot(GraphState &state, Window &window) : m_state(state), m_window(window)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TSSample) * COLS_MAX, nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TSSample), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 1, GL_FLOAT, GL_FALSE, sizeof(TSSample), (void *)offsetof(TSSample, min));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        2, 1, GL_FLOAT, GL_FALSE, sizeof(TSSample), (void *)offsetof(TSSample, max));
    glEnableVertexAttribArray(2);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("plot/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("plot/fragment.glsl"), GL_FRAGMENT_SHADER),
        Shader(Resources::find_shader("plot/geometry.glsl"), GL_GEOMETRY_SHADER)};
    m_shader = Program(shaders);
}

Plot::~Plot()
{
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
}

Plot::Plot(Plot &&other)
    : m_state(other.m_state), m_window(other.m_window), m_shader(other.m_shader)
{
    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    other.m_vao = 0;
    other.m_vbo = 0;
}

glm::dvec2 Plot::position() const
{
    return m_position;
}

void Plot::set_position(const glm::dvec2 &position)
{
    m_position = position;
}

glm::dvec2 Plot::size() const
{
    return m_size;
}

void Plot::set_size(const glm::dvec2 &size)
{
    m_size = size;
}

void Plot::draw_plot(const Window &window,
                     const std::vector<TSSample> &data,
                     glm::vec3 line_colour,
                     float y_offset) const
{
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glm::mat3 view_matrix_offset =
        glm::translate(m_state.view.matrix(), glm::dvec2(0.0f, y_offset));

    m_shader.use();
    int uniform_id = m_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(view_matrix_offset[0]));

    uniform_id = m_shader.uniform_location("viewport_matrix");
    const auto viewport_matrix = glm::mat3(window.viewport_transform().matrix());
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(viewport_matrix[0]));

    uniform_id = m_shader.uniform_location("viewport_matrix_inv");
    const auto viewport_matrix_inv = glm::mat3(window.viewport_transform().matrix_inverse());
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(viewport_matrix_inv[0]));

    uniform_id = m_shader.uniform_location("line_thickness_px");
    glUniform1i(uniform_id, m_state.plot_width);

    uniform_id = m_shader.uniform_location("show_line_segments");
    glUniform1i(uniform_id, m_state.show_line_segments);

    uniform_id = m_shader.uniform_location("plot_colour");
    glUniform3f(uniform_id, line_colour.r, line_colour.g, line_colour.b);

    uniform_id = m_shader.uniform_location("minmax_colour");
    glUniform3f(uniform_id, line_colour.r, line_colour.g, line_colour.b);

    // glScissor coordinates start in the bottom left
    glEnable(GL_SCISSOR_TEST);
    window.scissor(m_position.x, m_position.y, m_size.x, m_size.y);

    // Limit the number of samples we copy to the vertex buffer
    const auto num_samples = std::min(data.size(), COLS_MAX);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TSSample) * num_samples, data.data());
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, num_samples);
    glDisable(GL_SCISSOR_TEST);
}

void Plot::draw(const Window &window) const
{
    const auto plot_size_px = m_size;
    const auto plot_position_px = m_position;

    const int num_samples = plot_size_px.x / PIXELS_PER_COL;

    const auto plot_position_gs = screen2graph(plot_position_px);
    const auto plot_size_gs = screen2graph_delta(plot_size_px);
    const auto interval_gs = PIXELS_PER_COL * plot_size_gs.x / num_samples;

    for (auto &time_series : m_state.timeseries)
    {
        if (time_series.visible)
        {
            std::vector<TSSample> samples(num_samples);
            auto n_samples = time_series.ts->get_samples(
                samples.data(), plot_position_gs.x, interval_gs, num_samples);
            samples.resize(n_samples);

            draw_plot(window, samples, time_series.colour, time_series.y_offset);
        }
    }
}

void Plot::on_scroll(Window &window, double, double yoffset)
{
    on_zoom(window, yoffset);
}

void Plot::on_mouse_button(const glm::dvec2 &, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        m_is_dragging = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        m_is_dragging = false;
    }
}

void Plot::on_cursor_move(Window &window, double x, double y)
{
    const glm::dvec2 cursor_pos(x, y);
    if (m_is_dragging)
    {
        const auto delta = cursor_pos - m_cursor_pos_old;
        spdlog::info("{}{}", delta.x, delta.y);
        on_pan(window, delta);
    }
    m_cursor_pos_old = cursor_pos;
}

glm::dvec2 Plot::screen2graph(const glm::dvec2 &viewport_space) const
{
    const auto clip_space = m_window.viewport_transform().apply_inverse(viewport_space);
    const auto graph_space = m_state.view.apply_inverse(clip_space);
    return graph_space;
}

glm::dvec2 Plot::screen2graph_delta(const glm::dvec2 &delta) const
{
    auto begin_gs = screen2graph(glm::dvec2(0, 0));
    auto end_gs = screen2graph(glm::dvec2(0, 0) + delta);
    return end_gs - begin_gs;
}

glm::dvec2 Plot::graph2screen(const glm::dvec2 &value) const
{
    const auto clip_space = m_state.view.apply(value);
    const auto screen_space = m_window.viewport_transform().apply(clip_space);
    return screen_space;
}
