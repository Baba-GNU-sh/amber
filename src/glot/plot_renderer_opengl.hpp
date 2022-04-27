#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include "timeseries.hpp"
#include "window.hpp"

/**
 * @brief Draws plots in OpenGL.
 */
class PlotRendererOpenGL
{
  public:
    PlotRendererOpenGL(Window &window);
    ~PlotRendererOpenGL();
    void draw(const glm::mat3 &view_matrix,
              const std::vector<TSSample> &data,
              int plot_width,
              glm::vec3 plot_colour,
              float y_offset,
              bool show_line_segments,
              const glm::ivec2 &position,
              const glm::ivec2 &size) const;

  private:
    static constexpr size_t COLS_MAX = 8192; // Number of preallocated buffer space for samples
    Window &m_window;
    unsigned int m_plot_vao;
    unsigned int m_plot_vbo;
    Program m_plot_shader;
};
