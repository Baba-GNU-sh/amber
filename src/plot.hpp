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

  private:
    const glm::mat3x3 &_view_matrix;
	GLuint _plot_vao;
	GLuint _plot_vbo;
	Program _lines_shader;
	const int SAMPLE_COUNT = 4096;
};
