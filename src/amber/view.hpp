#pragma once

// TODO: Remove this dependency
// #include <cstddef>
#include <vector>
#include <glm/glm.hpp>
#include "hitbox.hpp"

enum class Action
{
    Unknown,

    Press,
    Release,
    Repeat
};

enum class MouseButton
{
    Unkown,

    Primary,
    Secondary,
    Middle
};

enum class Modifiers : int
{
    None = 0x0000,
    Shift = 0x0001,
    Alt = 0x0002,
    Control = 0x0004,
    Super = 0x0008
};

inline constexpr bool operator&(Modifiers x, Modifiers y)
{
    return static_cast<int>(x) & static_cast<int>(y);
}

inline constexpr Modifiers operator|(Modifiers x, Modifiers y)
{
    return static_cast<Modifiers>(static_cast<int>(x) | static_cast<int>(y));
}

enum class Key
{
    /* The unknown key */
    UNKNOWN = -1,

    /* Printable keys */
    SPACE = 32,
    APOSTROPHE = 39 /* ' */,
    COMMA = 44 /* , */,
    MINUS = 45 /* - */,
    PERIOD = 46 /* . */,
    SLASH = 47 /* / */,
    Num0 = 48,
    Num1 = 49,
    Num2 = 50,
    Num3 = 51,
    Num4 = 52,
    Num5 = 53,
    Num6 = 54,
    Num7 = 55,
    Num8 = 56,
    Num9 = 57,
    SEMICOLON = 59 /* ; */,
    EQUAL = 61 /* = */,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LEFT_BRACKET = 91 /* [ */,
    BACKSLASH = 92 /* \ */,
    RIGHT_BRACKET = 93 /* ] */,
    GRAVE_ACCENT = 96 /* ` */,
    WORLD_1 = 161 /* non-US #1 */,
    WORLD_2 = 162 /* non-US #2 */,

    /* Function keys */
    ESCAPE = 256,
    ENTER = 257,
    TAB = 258,
    BACKSPACE = 259,
    INSERT = 260,
    DELETE = 261,
    RIGHT = 262,
    LEFT = 263,
    DOWN = 264,
    UP = 265,
    PAGE_UP = 266,
    PAGE_DOWN = 267,
    HOME = 268,
    END = 269,
    CAPS_LOCK = 280,
    SCROLL_LOCK = 281,
    NUM_LOCK = 282,
    PRINT_SCREEN = 283,
    PAUSE = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    KP_0 = 320,
    KP_1 = 321,
    KP_2 = 322,
    KP_3 = 323,
    KP_4 = 324,
    KP_5 = 325,
    KP_6 = 326,
    KP_7 = 327,
    KP_8 = 328,
    KP_9 = 329,
    KP_DECIMAL = 330,
    KP_DIVIDE = 331,
    KP_MULTIPLY = 332,
    KP_SUBTRACT = 333,
    KP_ADD = 334,
    KP_ENTER = 335,
    KP_EQUAL = 336,
    LEFT_SHIFT = 340,
    LEFT_CONTROL = 341,
    LEFT_ALT = 342,
    LEFT_SUPER = 343,
    RIGHT_SHIFT = 344,
    RIGHT_CONTROL = 345,
    RIGHT_ALT = 346,
    RIGHT_SUPER = 347,
    MENU = 348,
};

struct View
{
    virtual ~View() = default;
    virtual void draw();
    virtual void on_scroll(const glm::dvec2 &cursor_position, double xoffset, double yoffset);
    virtual void on_mouse_button(const glm::dvec2 &cursor_pos,
                                 MouseButton button,
                                 Action action,
                                 Modifiers mods);
    virtual void on_cursor_move(double xpos, double ypos);
    virtual void on_key(Key key, int scancode, Action action, Modifiers mods);
    virtual void on_resize(int width, int height);
    virtual glm::dvec2 position() const;
    virtual void set_position(const glm::dvec2 &);
    virtual glm::dvec2 size() const;
    virtual void set_size(const glm::dvec2 &);
    virtual Hitbox hitbox() const;
    void add_view(View *view);

    std::vector<View *> m_views;
    View *m_sticky_view = nullptr;
    glm::vec2 m_size;
    glm::vec2 m_position;
};
