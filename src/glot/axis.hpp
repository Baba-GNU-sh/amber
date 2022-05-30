#pragma once

#include <boost/signals2.hpp>

#include "shader_utils.hpp"
#include "view.hpp"
#include "utils/transform.hpp"
#include "font.hpp"
#include "label.hpp"

class AxisBase : public View
{
  public:
    AxisBase(const Window &window);
    ~AxisBase();

    void draw(const Window &window) override;

    glm::dvec2 position() const override;
    glm::dvec2 size() const override;

    void set_position(const glm::dvec2 &position) override;
    void set_size(const glm::dvec2 &size) override;
    void set_graph_transform(const Transform<double> &t);

    void on_scroll(Window &, double x, double y) override;
    void on_mouse_button(const glm::dvec2 &cursor_pos, int button, int action, int mods) override;

    boost::signals2::signal<void(const Window &, double)> on_zoom;

  protected:
    std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> tick_spacing() const;
    glm::dvec2 screen2graph(const glm::dvec2 &value) const;
    glm::dvec2 screen2graph_delta(const glm::dvec2 &value) const;
    glm::dvec2 graph2screen(const glm::dvec2 &value) const;

    static glm::dvec2 crush(const glm::dvec2 &value, const glm::dvec2 &interval, bool ceil);

    void draw_ticks(const Window &window) const;

    void draw_labels(const Window &window);
    virtual void draw_ticks(const glm::dvec2 &tick_spacing,
                            double tick_size,
                            glm::vec2 *const ptr,
                            int &offset) const = 0;

    /**
     * @brief Re-lays out all the components. Call whenever buffers need to be updated.
     */
    virtual void update_layout() = 0;

    // Settings
    static constexpr double TICKLEN_PX = 5.0;
    static constexpr size_t NUM_LABELS = 128;

    // View invariates
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

enum AxisOrientation
{
    AxisHorizontal,
    AxisVertical
};

template <AxisOrientation Orientation> class Axis : public AxisBase
{
  public:
    Axis(const Window &window);

  private:
    void draw_ticks(const glm::dvec2 &tick_spacing,
                    double tick_size,
                    glm::vec2 *const ptr,
                    int &offset) const override;

    void update_layout() override;
};
