#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include <database/timeseries.hpp>
#include "window.hpp"

class Plot
{
  public:
    Plot(Window &window);
    ~Plot();
    Plot(const Plot &) = delete;
    Plot &operator=(const Plot &) = delete;
    Plot(Plot &&);
    Plot &operator=(Plot &&) = delete;

    void set_position(const glm::ivec2 &position);
    void set_size(const glm::ivec2 &size);

    void draw(const glm::mat3 &view_matrix,
              const std::vector<TSSample> &data,
              int plot_width,
              glm::vec3 plot_colour,
              float y_offset,
              bool show_line_segments) const;

  private:
    static constexpr size_t COLS_MAX = 8192; // Number of preallocated buffer space for samples
    Window &m_window;
    unsigned int m_vao;
    unsigned int m_vbo;
    Program m_shader;
    glm::ivec2 m_position;
    glm::ivec2 m_size;
};
