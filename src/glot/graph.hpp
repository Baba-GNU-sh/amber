#pragma once

#include <glm/glm.hpp>
#include "marker.hpp"
#include "plot.hpp"
#include "selection_box.hpp"
#include "window.hpp"
#include "graph_state.hpp"
#include "view.hpp"
#include "axis.hpp"

class Graph : public View
{
  public:
    enum class MarkerType
    {
        A,
        B
    };

    Graph(GraphState &state, Window &window);
    ~Graph() = default;
    Graph(const Graph &) = delete;
    Graph(Graph &&) = delete;
    Graph &operator=(const Graph &) = delete;
    Graph &operator=(Graph &&) = delete;

    glm::dvec2 cursor_gs() const;
    void draw(const Window &window) override;
    void on_resize(int, int) override;

    glm::dvec2 size() const override;
    glm::dvec2 position() const override;
    void on_mouse_button(const glm::dvec2 &cursor_pos, int button, int action, int mods) override;
    void on_cursor_move(double xpos, double ypos) override;
    void set_follow_latest_data(bool value);

    bool marker_is_visible(MarkerType m) const;
    double marker_position(MarkerType m) const;
    void set_marker_visible(MarkerType m, bool marker);
    void set_marker_position(MarkerType m, double position);
    void reveal_newest_sample();
    double latest_visibile_sample_time() const;
    const Transform<double> &get_view_transform() const;

  private:
    void layout();

    glm::dvec2 screen2graph(const glm::dvec2 &value) const;
    glm::dvec2 screen2graph_delta(const glm::dvec2 &value) const;
    glm::dvec2 graph2screen(const glm::dvec2 &value) const;

    void apply_zoom(const glm::dvec2 &);

    const Marker &get_marker(MarkerType m) const
    {
        switch (m)
        {
        default:
        case MarkerType::A:
            return m_marker_a;
        case MarkerType::B:
            return m_marker_b;
        }
    }

    Marker &get_marker(MarkerType m)
    {
        switch (m)
        {
        default:
        case MarkerType::A:
            return m_marker_a;
        case MarkerType::B:
            return m_marker_b;
        }
    }

    void fit_graph(const glm::dvec2 &tl, const glm::dvec2 &br);

    static constexpr double GUTTER_SIZE = 60;
    static constexpr double ZOOM_MAX = 10e6;
    static constexpr int PIXELS_PER_COL = 1;

    Transform<double> m_view;
    GraphState &m_state;
    Window &m_window;
    Axis<AxisHorizontal> m_axis_horizontal;
    Axis<AxisVertical> m_axis_vertical;
    Plot m_plot;
    Marker m_marker_a;
    Marker m_marker_b;
    SelectionBox m_selection_box;

    glm::dvec2 m_size;
    glm::dvec2 m_position;

    bool m_is_selecting = false;
    glm::dvec2 m_selection_start;
    bool m_follow_latest_data = false;
};
