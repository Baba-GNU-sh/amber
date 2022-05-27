#pragma once

#include "window.hpp"

struct View
{
    virtual ~View() = default;
    virtual void draw(const Window &window) const = 0;

    virtual void on_scroll(Window &, double, double)
    {
        //
    }

    virtual void on_mouse_button(Window &, int, int, int)
    {
        //
    }

    virtual void on_cursor_move(Window &, double, double)
    {
        //
    }

    virtual void on_key(Window &, int, int, int, int)
    {
        //
    }

    virtual void on_resize(int, int)
    {
        //
    }

    virtual glm::dvec2 position() const
    {
        return glm::dvec2(0.0);
    }

    virtual void set_position(const glm::dvec2 &)
    {
        //
    }

    virtual glm::dvec2 size() const
    {
        return glm::dvec2(0.0);
    }

    virtual void set_size(const glm::dvec2 &)
    {
        //
    }
};
