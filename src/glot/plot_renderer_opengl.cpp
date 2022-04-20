#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "plot_renderer_opengl.hpp"
#include "timeseries.hpp"
#include "resources.hpp"

Plot::Plot(Window &window) : m_window(window)
{
    glGenVertexArrays(1, &_plot_vao);
    glBindVertexArray(_plot_vao);

    glGenBuffers(1, &_plot_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _plot_vbo);
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

    _lines_shader = Program(shaders);
}

Plot::~Plot()
{
    glDeleteVertexArrays(1, &_plot_vao);
    glDeleteBuffers(1, &_plot_vbo);
}

void Plot::draw(const glm::mat3 &view_matrix,
                const TimeSeries &ts,
                int line_width,
                glm::vec3 line_colour,
                float y_offset,
                bool show_line_segments,
                const glm::ivec2 &position,
                const glm::ivec2 &size) const
{
    glBindVertexArray(_plot_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _plot_vbo);

    auto view_matrix_inv = glm::inverse(view_matrix);

    glm::mat3 view_matrix_offset = glm::translate(view_matrix, glm::vec2(0.0f, y_offset));

    _lines_shader.use();
    int uniform_id = _lines_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(view_matrix_offset[0]));

    uniform_id = _lines_shader.uniform_location("viewport_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(m_window.vp_matrix()[0]));

    uniform_id = _lines_shader.uniform_location("viewport_matrix_inv");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(m_window.vp_matrix_inv()[0]));

    uniform_id = _lines_shader.uniform_location("line_thickness_px");
    glUniform1i(uniform_id, line_width);

    uniform_id = _lines_shader.uniform_location("show_line_segments");
    glUniform1i(uniform_id, show_line_segments);

    uniform_id = _lines_shader.uniform_location("plot_colour");
    glUniform3f(uniform_id, line_colour.r, line_colour.g, line_colour.b);

    uniform_id = _lines_shader.uniform_location("minmax_colour");
    glUniform3f(uniform_id, line_colour.r, line_colour.g, line_colour.b);

    // Pull out samples binned by vertical columns of pixels
    const int width = std::min(size.x / PIXELS_PER_COL, COLS_MAX);

    // Work out where on the graph the first column of pixels lives
    glm::vec3 begin(position.x, 0.0f, 1.0f);
    auto begin_gs = view_matrix_inv * m_window.vp_matrix_inv() * begin;

    glm::vec3 end(position.x + width, 0.0f, 1.0f);
    auto end_gs = view_matrix_inv * m_window.vp_matrix_inv() * end;

    auto interval = PIXELS_PER_COL * (end_gs.x - begin_gs.x) / width;

    auto n_samples = ts.get_samples(&_samples[0], begin_gs.x, interval, width);

    // glScissor uses coordinates starting at the bottom left
    glEnable(GL_SCISSOR_TEST);
    const auto window_size = m_window.size();
    glScissor(position.x, window_size.y - (position.y + size.y), size.x, size.y);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TSSample) * n_samples, _samples);
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, n_samples);
    glDisable(GL_SCISSOR_TEST);
}
