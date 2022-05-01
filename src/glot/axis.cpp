#include "axis.hpp"
#include <glm/fwd.hpp>

Axis::Axis(Window &window, GraphState &state, Orientation orientation)
    : m_window(window), m_state(state), m_orientation(orientation), m_font("proggy_clean.png")
{
    for (int i = 0; i < LABELS_MAX; ++i)
    {
        m_labels.emplace_back(m_window, m_font, 32);
    }
}

Axis::~Axis()
{
    // TODO
}

void Axis::set_size(const glm::ivec2 &size)
{
    m_size = size;
}

void Axis::set_position(const glm::ivec2 &position)
{
    m_position = position;
}

void Axis::draw()
{
    using namespace glm;

    const auto start = m_position;
    const auto end = m_orientation == Orientation::Horizontal ? m_position + ivec2(m_size.x, 0)
                                                              : m_position + ivec2(0, m_size.y);
    const auto tick_size_major =
        m_orientation == Orientation::Horizontal ? ivec2(0, TICKLEN_MAJOR_PX) : ivec2(TICKLEN_MAJOR_PX, 0);
    const auto tick_size_minor =
        m_orientation == Orientation::Horizontal ? ivec2(0, TICKLEN_MINOR_PX) : ivec2(TICKLEN_MINOR_PX, 0);


}
