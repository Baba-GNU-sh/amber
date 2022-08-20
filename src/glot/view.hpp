#pragma once

// TODO: Remove this dependency
#include <cstddef>
#include <glm/glm.hpp>
#include <vector>

struct HitBox
{
    glm::dvec2 tl;
    glm::dvec2 br;
};

struct View
{
    virtual ~View() = default;
    virtual void draw();
    virtual void on_scroll(const glm::dvec2 &cursor_position, double xoffset, double yoffset);
    virtual void on_mouse_button(const glm::dvec2 &cursor_pos, int button, int action, int mods);
    virtual void on_cursor_move(double xpos, double ypos);
    virtual void on_key(int key, int scancode, int action, int mods);
    virtual void on_resize(int width, int height);
    virtual glm::dvec2 position() const;
    virtual void set_position(const glm::dvec2 &);
    virtual glm::dvec2 size() const;
    virtual void set_size(const glm::dvec2 &);
    virtual HitBox get_hitbox() const;
    void add_view(View *view);

    std::vector<View *> m_views;
    View *m_sticky_view = nullptr;
};
