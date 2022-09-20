#include <glm/glm.hpp>

struct Hitbox
{
    glm::dvec2 tl;
    glm::dvec2 br;

    bool test(double x, double y);
    bool test(const glm::dvec2 &point);
};
