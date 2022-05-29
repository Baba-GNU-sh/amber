#pragma once

#include <glm/glm.hpp>
#include "utils/transform.hpp"

struct Window
{
    virtual glm::dvec2 cursor() const = 0;
    virtual const Transform<double> &viewport_transform() const = 0;
    virtual void scissor(int x, int y, int width, int height) const = 0;
    virtual void request_close() = 0;
    virtual bool should_close() const = 0;
    virtual void set_fullscreen(bool fullscreen) = 0;
    virtual bool is_fullscreen() const = 0;
    virtual void set_bg_colour(const glm::vec3 &colour) = 0;
    virtual glm::ivec2 window_size() const = 0;
};
