#pragma once

#include <glad/glad.h> // Keep this one before glfw to avoid errors
#include <glm/glm.hpp>

#include "shader_utils.hpp"

class Plot
{
  public:
	Plot(const glm::mat3x3 &view_matrix);
	~Plot();
    void draw() const;

	/**
	 * @brief Update the matrix describing the transform from screen space to pixels.
	 *
	 * @param viewport_matrix The new value of the viewport matrix.
	 */
	void update_viewport_matrix(const glm::mat3x3& viewport_matrix);

	int *get_line_thickness();
	glm::vec3 *get_plot_colour();
	glm::vec3 *get_minmax_colour();
	bool *get_show_line_segments();

  private:
    const glm::mat3x3 &_view_matrix;
	glm::mat3x3 _viewport_matrix;
	glm::mat3x3 _viewport_matrix_inv;
	GLuint _plot_vao;
	GLuint _plot_vbo;
	Program _lines_shader;
	static constexpr int SAMPLE_COUNT = 4096;
	int _line_thickness_px;
	glm::vec3 _plot_colour;
	glm::vec3 _minmax_colour;
	bool _show_line_segments = false;
};
