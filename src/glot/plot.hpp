#pragma once

#include <glm/glm.hpp>
#include "shader_utils.hpp"
#include <database/timeseries.hpp>
#include "window.hpp"
#include "view.hpp"
#include <boost/signals2.hpp>
#include "graph_state.hpp"

class Plot : public View
{
  public:
    Plot(GraphState &state);
    ~Plot();
    Plot(const Plot &) = delete;
    Plot &operator=(const Plot &) = delete;
    Plot(Plot &&);
    Plot &operator=(Plot &&) = delete;

    glm::dvec2 position() const override;
    void set_position(const glm::dvec2 &position) override;

    glm::dvec2 size() const override;
    void set_size(const glm::dvec2 &size) override;

    void draw(const Window &window) const override;

    void draw_plot(const Window &window,
                   const std::vector<TSSample> &data,
                   glm::vec3 plot_colour,
                   float y_offset) const;

    void on_scroll(const Window &, double, double) override;

    boost::signals2::signal<void(const Window &, double)> on_zoom;

  private:
    glm::dvec2 screen2graph(const Transform<double> &viewport_txform,
                            const glm::ivec2 &value) const;
    glm::dvec2 screen2graph_delta(const Transform<double> &viewport_txform,
                                  const glm::ivec2 &value) const;
    glm::dvec2 graph2screen(const Transform<double> &viewport_txform,
                            const glm::dvec2 &value) const;

    GraphState &m_state;
    static constexpr size_t PIXELS_PER_COL = 1;
    static constexpr size_t COLS_MAX = 8192; // Number of preallocated buffer space for samples
    unsigned int m_vao;
    unsigned int m_vbo;
    Program m_shader;
    glm::dvec2 m_position;
    glm::dvec2 m_size;
};
