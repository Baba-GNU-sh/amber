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
  : _view_matrix(view_matrix)
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

    uniform_id = _lines_shader.get_uniform_location("viewport_res_px");
    // glm::ivec2 sz(1.0);
    // glfwGetWindowSize(win, &sz.x, &sz.y);
	glUniform2i(uniform_id, 800, 600);

    uniform_id = _lines_shader.get_uniform_location("line_thickness_px");
    // glm::ivec2 sz(1.0);
    // glfwGetWindowSize(win, &sz.x, &sz.y);
	glUniform1f(uniform_id, 3.0f);

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
