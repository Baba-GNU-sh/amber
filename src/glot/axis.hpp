#pragma once

#include <boost/signals2.hpp>

#include "shader_utils.hpp"
#include "view.hpp"
#include "utils/transform.hpp"

class Axis : public View
{
  public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    // TODO: can we make this orientation setting a template parameter?
    Axis(Orientation ori, const Transform<double> &graph_transform);
    ~Axis();

    void draw(const Window &window) const override;

    glm::dvec2 position() const override;
    void set_position(const glm::dvec2 &position) override;
    glm::dvec2 size() const override;
    void set_size(const glm::dvec2 &size) override;
    void set_orientation(Orientation ori);
    void on_scroll(Window &, double x, double y) override;
    void on_mouse_button(Window &, int button, int action, int mods) override;

    boost::signals2::signal<void(const Window &, double)> on_zoom;

  private:
    std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> tick_spacing(
        const Transform<double> &viewport_transform) const;
    glm::dvec2 screen2graph(const Transform<double> &viewport_txform,
                            const glm::ivec2 &value) const;
    glm::dvec2 screen2graph_delta(const Transform<double> &viewport_txform,
                                  const glm::ivec2 &value) const;
    glm::dvec2 graph2screen(const Transform<double> &viewport_txform,
                            const glm::dvec2 &value) const;

    static glm::dvec2 crush(const glm::dvec2 &value, const glm::dvec2 &interval);
    void draw_ticks(const glm::dvec2 &tick_spacing,
                    double tick_size,
                    glm::vec2 *const ptr,
                    int &offset,
                    const Transform<double> &vpt) const;

    Orientation m_orientation;
    const Transform<double> &m_graph_transform;
    glm::dvec2 m_position = glm::dvec2(0.0);
    glm::dvec2 m_size = glm::dvec2(100.0);

    unsigned int m_linebuf_vao;
    unsigned int m_linebuf_vbo;
    Program m_lines_shader;

    static constexpr double TICKLEN_PX = 5.0;
};
