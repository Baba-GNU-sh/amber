#include "plot.hpp"

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Sample
{
    float x;
    float y;
    float min;
    float max;
};

Plot::Plot(const glm::mat3x3& view_matrix)
  : _view_matrix(view_matrix),
    _line_thickness_px(2.0)
{
	glGenVertexArrays(1, &_plot_vao);
	glBindVertexArray(_plot_vao);

	glGenBuffers(1, &_plot_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _plot_vbo);
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(Sample) * SAMPLE_COUNT,
	             nullptr,
	             GL_STREAM_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Sample), (void*)0);
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Sample), (void*)offsetof(Sample, min));
	glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Sample), (void*)offsetof(Sample, max));
	glEnableVertexAttribArray(2);

	std::vector<Shader> shaders{
        Shader("vertex.glsl", GL_VERTEX_SHADER),
        Shader("fragment.glsl", GL_FRAGMENT_SHADER),
        Shader("geometry.glsl", GL_GEOMETRY_SHADER)
    };

	_lines_shader = Program(shaders);
}

Plot::~Plot()
{
	glDeleteVertexArrays(1, &_plot_vao);
	glDeleteBuffers(1, &_plot_vbo);
}

void
Plot::draw() const
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
    // glm::ivec2 sz(1.0);
    // glfwGetWindowSize(win, &sz.x, &sz.y);
	glUniform1i(uniform_id, _line_thickness_px);

    uniform_id = _lines_shader.get_uniform_location("multicoloured_line_segments");
    // glm::ivec2 sz(1.0);
    // glfwGetWindowSize(win, &sz.x, &sz.y);
	glUniform1i(uniform_id, false);

	auto time = glfwGetTime();
	Sample plot_data[SAMPLE_COUNT];
	for (int i = 0; i < SAMPLE_COUNT; i++)
    {
		const float x = static_cast<float>(i - SAMPLE_COUNT / 2) / 400.0f;
		plot_data[i].x = x;
		plot_data[i].y = sinf(100.0f * x + time) * sinf(1.0f * x);
        plot_data[i].min = plot_data[i].y - .1;
        plot_data[i].max = plot_data[i].y + .1;
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(plot_data), &plot_data);
	glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, SAMPLE_COUNT);
}

void Plot::update_viewport_matrix(const glm::mat3x3 &viewport_matrix)
{
    _viewport_matrix = viewport_matrix;
	_viewport_matrix_inv = glm::inverse(viewport_matrix);
}

void Plot::set_line_thickness(int line_thickness_px)
{
    _line_thickness_px = line_thickness_px;
}

int *Plot::get_line_thickness()
{
    return &_line_thickness_px;
}
