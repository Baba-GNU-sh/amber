#include <glm/glm.hpp>

namespace amber
{
template <class T> struct Hitbox
{
    typedef glm::vec<2, T> VecType;

    VecType tl;
    VecType br;

    bool test(T x, T y)
    {
        if (x < tl.x)
            return false;
        if (x > br.x)
            return false;
        if (y < tl.y)
            return false;
        if (y > br.y)
            return false;
        return true;
    }

    bool test(const VecType &point)
    {
        return test(point.x, point.y);
    }
};
} // namespace amber
