#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "plot_renderer_opengl.hpp"
#include <database/timeseries.hpp>
#include "resources.hpp"

PlotRendererOpenGL::PlotRendererOpenGL(Window &window) : m_window(window)
{
    glGenVertexArrays(1, &m_plot_vao);
    glBindVertexArray(m_plot_vao);

    glGenBuffers(1, &m_plot_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_plot_vbo);
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
    m_plot_shader = Program(shaders);
}

PlotRendererOpenGL::~PlotRendererOpenGL()
{
    glDeleteVertexArrays(1, &m_plot_vao);
    glDeleteBuffers(1, &m_plot_vbo);
}

void PlotRendererOpenGL::draw(const glm::mat3 &view_matrix,
                              const std::vector<TSSample> &data,
                              int line_width,
                              glm::vec3 line_colour,
                              float y_offset,
                              bool show_line_segments,
                              const glm::ivec2 &position,
                              const glm::ivec2 &size) const
{
    glBindVertexArray(m_plot_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_plot_vbo);

    glm::mat3 view_matrix_offset = glm::translate(view_matrix, glm::vec2(0.0f, y_offset));

    m_plot_shader.use();
    int uniform_id = m_plot_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(view_matrix_offset[0]));

    uniform_id = m_plot_shader.uniform_location("viewport_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(m_window.vp_matrix()[0]));

    uniform_id = m_plot_shader.uniform_location("viewport_matrix_inv");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(m_window.vp_matrix_inv()[0]));

    uniform_id = m_plot_shader.uniform_location("line_thickness_px");
    glUniform1i(uniform_id, line_width);

    uniform_id = m_plot_shader.uniform_location("show_line_segments");
    glUniform1i(uniform_id, show_line_segments);

    uniform_id = m_plot_shader.uniform_location("plot_colour");
    glUniform3f(uniform_id, line_colour.r, line_colour.g, line_colour.b);

    uniform_id = m_plot_shader.uniform_location("minmax_colour");
    glUniform3f(uniform_id, line_colour.r, line_colour.g, line_colour.b);

    // glScissor coordinates start in the bottom left
    glEnable(GL_SCISSOR_TEST);
    const auto window_size = m_window.size();
    glScissor(position.x, window_size.y - (position.y + size.y), size.x, size.y);

    // Limit the number of samples we copy to the vertex buffer
    const auto num_samples = std::min(data.size(), COLS_MAX);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TSSample) * num_samples, data.data());
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, num_samples);
    glDisable(GL_SCISSOR_TEST);
}
