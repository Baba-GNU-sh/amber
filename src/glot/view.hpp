#pragma once

#include "window.hpp"

struct View
{
    virtual ~View() = default;
    virtual void draw(const Window &window) const = 0;

    virtual void on_scroll(double, double)
    {
        //
    }

    virtual void on_zoom(double, double)
    {
        //
    }

    virtual void on_mouse_button(int, int, int)
    {
        //
    }

    virtual void on_cursor_move(double, double)
    {
        //
    }

    virtual void on_key(int, int, int, int)
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
