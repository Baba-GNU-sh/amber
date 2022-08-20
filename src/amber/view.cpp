#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <algorithm>
#include "graph_utils.hpp"
#include "view.hpp"

void View::draw()
{
    std::for_each(m_views.begin(), m_views.end(), [](View *view) { view->draw(); });
}

void View::on_scroll(const glm::dvec2 &cursor_position, double xoffset, double yoffset)
{
    // Search views backward, which is the opposite of the render order
    // This means that views rendered last (on top) should be hit first
    for (auto iter = m_views.rbegin(); iter != m_views.rend(); iter++)
    {
        auto *view = *iter;
        const auto hitbox = view->get_hitbox();
        if (GraphUtils::hit_test(cursor_position, hitbox.tl, hitbox.br))
        {
            view->on_scroll(cursor_position, xoffset, yoffset);
            break;
        }
    }
}

void View::on_mouse_button(const glm::dvec2 &cursor_pos, int button, int action, int mods)
{
    // This logic gets a bit tricky
    // If a view is clicked, then the cursor moved outside of the view before the view is
    // released, we still want to send the release event to the view. How do we handle multiple
    // simultaneous button presses?
    if (action == GLFW_PRESS)
    {
        // Search views backward, which is the opposite of the render order
        // This means that views rendered last (on top) should be hit first
        for (auto iter = m_views.rbegin(); iter != m_views.rend(); iter++)
        {
            auto *view = *iter;
            const auto hitbox = view->get_hitbox();
            if (GraphUtils::hit_test(cursor_pos, hitbox.tl, hitbox.br))
            {
                view->on_mouse_button(cursor_pos, button, action, mods);
                m_sticky_view = view;
                break;
            }
        }
    }
    else if (action == GLFW_RELEASE)
    {
        if (m_sticky_view)
        {
            m_sticky_view->on_mouse_button(cursor_pos, button, action, mods);
            m_sticky_view = nullptr;
        }
        else
        {
            std::for_each(
                m_views.begin(), m_views.end(), [&cursor_pos, button, action, mods](auto *view) {
                    if (GraphUtils::hit_test(
                            cursor_pos, view->position(), view->position() + view->size()))
                    {
                        view->on_mouse_button(cursor_pos, button, action, mods);
                    }
                });
        }
    }
}

void View::on_cursor_move(double xpos, double ypos)
{
    std::for_each(m_views.begin(), m_views.end(), [xpos, ypos](auto *view) {
        view->on_cursor_move(xpos, ypos);
    });
}

void View::on_key(int key, int scancode, int action, int mods)
{
    std::for_each(m_views.begin(), m_views.end(), [key, scancode, action, mods](auto *view) {
        view->on_key(key, scancode, action, mods);
    });
}

void View::on_resize(int width, int height)
{
    std::for_each(m_views.begin(), m_views.end(), [width, height](auto *view) {
        view->on_resize(width, height);
    });
}

glm::dvec2 View::position() const
{
    return glm::dvec2(0.0);
}

void View::set_position(const glm::dvec2 &)
{
    //
}

glm::dvec2 View::size() const
{
    return glm::dvec2(0.0);
}

void View::set_size(const glm::dvec2 &)
{
    //
}

HitBox View::get_hitbox() const
{
    return HitBox{position(), position() + size()};
}

void View::add_view(View *view)
{
    m_views.push_back(view);
}
