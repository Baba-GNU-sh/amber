#include "hitbox.hpp"

bool Hitbox::test(const glm::dvec2 &point)
{
    return test(point.x, point.y);
}

bool Hitbox::test(double x, double y)
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
