#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "label.hpp"
#include "graph_state.hpp"
#include "window.hpp"
#include "label.hpp"

class Axis
{
  public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    Axis(Window &window, GraphState &state, Orientation orientation);
    ~Axis();

    Axis(const Axis &) = delete;
    Axis(Axis &&) = delete;
    Axis &operator=(const Axis &) = delete;
    Axis &operator=(Axis &&) = delete;

    void set_size(const glm::ivec2 &);
    void set_position(const glm::ivec2 &);
    void draw();

  private:
    static constexpr int LABELS_MAX = 64;
    static constexpr int TICKLEN_MAJOR_PX = 5;
    static constexpr int TICKLEN_MINOR_PX = 2;
    Window &m_window;
    GraphState &m_state;
    Orientation m_orientation;
    Font m_font;
    glm::ivec2 m_size;
    glm::ivec2 m_position;
    std::vector<Label> m_labels;
};
