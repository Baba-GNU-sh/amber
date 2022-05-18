#pragma once

#include <glm/glm.hpp>

class GraphUtils
{
  public:
    static bool hit_test(glm::dvec2 value, glm::dvec2 tl, glm::dvec2 br)
    {
        if (value.x < tl.x)
            return false;
        if (value.x > br.x)
            return false;
        if (value.y < tl.y)
            return false;
        if (value.y > br.y)
            return false;
        return true;
    }
};
