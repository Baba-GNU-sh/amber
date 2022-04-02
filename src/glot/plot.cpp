#include "plot.hpp"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.hpp"
#include "timeseries.hpp"

Plot::Plot(const glm::mat3x3 &view_matrix)
    : _view_matrix(view_matrix), _line_thickness_px(2.0), _plot_colour(1.0f, 0.5f, 0.2f),
      _minmax_colour(0.5f, 0.5f, 0.5f)
{
    glGenVertexArrays(1, &_plot_vao);
    glBindVertexArray(_plot_vao);

    glGenBuffers(1, &_plot_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _plot_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TSSample) * COLS_MAX, nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Sample), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Sample), (void *)offsetof(Sample, min));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Sample), (void *)offsetof(Sample, max));
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

void Plot::draw(const TimeSeries &ts) const
{
    glBindVertexArray(_plot_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _plot_vbo);

    _lines_shader.use();
    int uniform_id = _lines_shader.get_uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(_view_matrix[0]));

    uniform_id = _lines_shader.get_uniform_location("viewport_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(_viewport_matrix[0]));

    uniform_id = _lines_shader.get_uniform_location("viewport_matrix_inv");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(_viewport_matrix_inv[0]));

    uniform_id = _lines_shader.get_uniform_location("line_thickness_px");
    glUniform1i(uniform_id, _line_thickness_px);

    uniform_id = _lines_shader.get_uniform_location("show_line_segments");
    glUniform1i(uniform_id, _show_line_segments);

    uniform_id = _lines_shader.get_uniform_location("plot_colour");
    glUniform3f(uniform_id, _plot_colour.r, _plot_colour.g, _plot_colour.b);
    uniform_id = _lines_shader.get_uniform_location("minmax_colour");
    glUniform3f(uniform_id, _minmax_colour.r, _minmax_colour.g, _minmax_colour.b);

    // Pull out samples binned by vertical columns of pixels
    const int PIXELS_PER_COL = 2;
    const int width = std::min(_size.x / PIXELS_PER_COL, COLS_MAX);

    // Work out where on the graph the first column of pixels lives
    glm::vec3 begin(0.0f, 0.0f, 1.0f);
    auto begin_gs = glm::inverse(_view_matrix) * _viewport_matrix_inv * begin;

    glm::vec3 end(width, 0.0f, 1.0f);
    auto end_gs = glm::inverse(_view_matrix) * _viewport_matrix_inv * end;

    auto interval = PIXELS_PER_COL * (end_gs.x - begin_gs.x) / width;

    TSSample samples[width];
    auto n_samples = ts.get_samples(&samples[0], begin_gs.x, interval, width);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TSSample) * n_samples, samples);
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, n_samples);
}

void Plot::update_viewport_matrix(const glm::mat3x3 &viewport_matrix)
{
    _viewport_matrix = viewport_matrix;
    _viewport_matrix_inv = glm::inverse(viewport_matrix);
}

void Plot::set_size(int width, int height)
{
    _size = glm::ivec2(width, height);
}

int *Plot::get_line_thickness()
{
    return &_line_thickness_px;
}

glm::vec3 *Plot::get_plot_colour()
{
    return &_plot_colour;
}

glm::vec3 *Plot::get_minmax_colour()
{
    return &_minmax_colour;
}

bool *Plot::get_show_line_segments()
{
    return &_show_line_segments;
}
