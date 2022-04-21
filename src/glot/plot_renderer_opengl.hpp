#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "timeseries.hpp"
#include "window.hpp"

/**
 * @brief Defines how to draw plots.
 */
class PlotRendererOpenGL
{
  public:
    PlotRendererOpenGL(Window &window);
    ~PlotRendererOpenGL();
    void draw(const glm::mat3 &view_matrix,
              const TimeSeries &ts,
              int plot_width,
              glm::vec3 plot_colour,
              float y_offset,
              bool show_line_segments,
              const glm::ivec2 &position,
              const glm::ivec2 &size) const;

  private:
    static constexpr int COLS_MAX =
        8192; // Max number of columns to allocate buffers for (enough to fill a DCI 8K monitor!)
    static constexpr int PIXELS_PER_COL = 1; // How wide the column are in pixels

    Window &m_window;
    unsigned int _plot_vao;
    unsigned int _plot_vbo;
    Program _lines_shader;
    mutable TSSample _samples[COLS_MAX];
};