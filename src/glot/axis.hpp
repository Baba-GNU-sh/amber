#pragma once

#include <boost/signals2.hpp>

#include "shader_utils.hpp"
#include "view.hpp"
#include "utils/transform.hpp"
#include "font.hpp"
#include "label.hpp"

class Axis : public View
{
  public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    // TODO: can we make this orientation setting a template parameter?
    Axis(Orientation orientation, const Window &window);
    ~Axis();

    void draw(const Window &window) const override;

    glm::dvec2 position() const override;
    glm::dvec2 size() const override;

    void set_position(const glm::dvec2 &position) override;
    void set_size(const glm::dvec2 &size) override;
    void set_graph_transform(const Transform<double> &t);

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

    void draw_ticks(const Window &window) const;
    void draw_labels(const Window &window) const;

    /**
     * @brief Re-lays out all the components. Call whenever buffers need to be updated.
     */
    void update_layout();

    // Settings
    static constexpr double TICKLEN_PX = 5.0;
    static constexpr size_t NUM_LABELS = 128;

    // View invariates
    const Orientation m_orientation; // TODO: make this a template parameter
    const Window &m_window;
    Font m_font;

    // View variables
    Transform<double> m_graph_transform;
    glm::dvec2 m_position = glm::dvec2(0.0);
    glm::dvec2 m_size = glm::dvec2(100.0);
    std::vector<Label> m_labels;
    size_t m_labels_used;

    unsigned int m_linebuf_vao;
    unsigned int m_linebuf_vbo;
    Program m_lines_shader;
};
