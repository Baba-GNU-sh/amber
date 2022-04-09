#pragma once

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
    Plot(const glm::mat3 &view_matrix);
    ~Plot();
    void draw(const TimeSeries &ts,
              const glm::mat3 &vp_matrix,
              int plot_width,
              glm::vec3 plot_colour,
              float y_offset,
              bool show_line_segments) const;
    void set_size(int width, int height);

  private:
    const glm::mat3 &_view_matrix;
    unsigned int _plot_vao;
    unsigned int _plot_vbo;
    Program _lines_shader;
    static constexpr int COLS_MAX = 8000; // This could come back to bit me!
    glm::ivec2 _size;
    mutable TSSample _samples[COLS_MAX];
};
