#pragma once

#include <glad/glad.h> // Keep this one before glfw to avoid errors
#include <glm/glm.hpp>

#include "shader_utils.hpp"
#include "timeseries.hpp"

struct Sample
{
    float x;
    float y;
    float min;
    float max;
};

class Plot
{
  public:
    Plot(const glm::mat3x3 &view_matrix);
    ~Plot();
    void draw(const TimeSeries &ts,
              int plot_width,
              glm::vec3 plot_colour,
              glm::vec3 minmax_colour,
              bool show_line_segments) const;
    void update_viewport_matrix(const glm::mat3x3 &viewport_matrix);
    void set_size(int width, int height);

  private:
    const glm::mat3x3 &_view_matrix;
    glm::mat3x3 _viewport_matrix;
    glm::mat3x3 _viewport_matrix_inv;
    GLuint _plot_vao;
    GLuint _plot_vbo;
    Program _lines_shader;
    static constexpr int COLS_MAX = 8000; // This could come back to bit me!
    std::vector<Sample> _samples;
    glm::ivec2 _size;
};
