#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <algorithm>

/**
 * @brief Stores 2D transformation.
 *
 * @tparam T The type of float to use in the underlying matrix e.g. double or float.
 */
template <class T> class Transform
{
    typedef glm::mat<3, 3, T> MatType;

  public:
    Transform()
    {
        //
    }

    Transform(const MatType &value)
    {
        update(value);
    }

    void set_zoom_limit(T limit)
    {
        m_zoom_limit = limit;
    }

    void update(const MatType &value)
    {
        m_transform = value;
        m_transform[0][0] = std::min(m_transform[0][0], m_zoom_limit);
        m_transform[1][1] = std::min(m_transform[1][1], m_zoom_limit);
        m_inverse = glm::inverse(value);
    }

    const MatType &matrix() const
    {
        return m_transform;
    }

    const MatType &matrix_inverse() const
    {
        return m_inverse;
    }

    glm::vec<2, T> apply(const glm::vec<2, T> &vec) const
    {
        return m_transform * glm::vec<3, T>(vec, 1.0);
    }

    glm::vec<2, T> apply_inverse(const glm::vec<2, T> &vec) const
    {
        return glm::vec<2, T>(m_inverse * glm::vec<3, T>(vec, 1.0));
    }

    void translate(const glm::dvec2 &delta)
    {
        update(glm::translate(m_transform, delta));
    }

    void scale(const glm::dvec2 &delta)
    {
        update(glm::scale(m_transform, delta));
    }

    MatType m_transform;
    MatType m_inverse;
    T m_zoom_limit = std::numeric_limits<T>::max();
};
