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

	void set_line_thickness(int line_thickness_px);
	int *get_line_thickness();

  private:
    const glm::mat3x3 &_view_matrix;
	glm::mat3x3 _viewport_matrix;
	glm::mat3x3 _viewport_matrix_inv;
	GLuint _plot_vao;
	GLuint _plot_vbo;
	Program _lines_shader;
	const int SAMPLE_COUNT = 4096;
	int _line_thickness_px;
};
