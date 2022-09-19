#include "view.hpp"
#include <glm/ext/vector_double2.hpp>
#include <glm/ext/vector_float2.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::StrictMock;

TEST(View, modifiers_can_be_combined)
{
    auto mods = Modifiers::Alt | Modifiers::Control;
    ASSERT_TRUE(mods & Modifiers::Alt);
    ASSERT_TRUE(mods & Modifiers::Control);
    ASSERT_FALSE(mods & Modifiers::Shift);
}

class MockView : public View
{
  public:
    MOCK_METHOD(void,
                on_mouse_button,
                (const glm::dvec2 &, MouseButton, Action, Modifiers),
                (override));
    MOCK_METHOD(void, on_scroll, (const glm::dvec2 &, double, double), (override));
    MOCK_METHOD(void, draw, (), (override));
    MOCK_METHOD(void, on_cursor_move, (double x, double y), (override));
    MOCK_METHOD(void, on_resize, (int, int), (override));
    MOCK_METHOD(void, on_key, (Key, int, Action action, Modifiers mods), (override));
};

TEST(View, on_mouse_button_call_forwarded_to_child_view_and_sticky)
{
    View view;

    MockView mock_view;
    view.add_view(&mock_view);

    mock_view.set_position(glm::dvec2(0, 0));
    mock_view.set_size(glm::dvec2(500, 500));

    glm::dvec2 cursor_inside(100, 200);
    glm::dvec2 cursor_outside(-100, -100);

    EXPECT_CALL(
        mock_view,
        on_mouse_button(cursor_inside, MouseButton::Primary, Action::Press, Modifiers::None))
        .Times(1);
    view.on_mouse_button(cursor_inside, MouseButton::Primary, Action::Press, Modifiers::None);

    EXPECT_CALL(
        mock_view,
        on_mouse_button(cursor_outside, MouseButton::Primary, Action::Release, Modifiers::None))
        .Times(1);
    view.on_mouse_button(cursor_outside, MouseButton::Primary, Action::Release, Modifiers::None);
}

TEST(View, on_mouse_button_call_forwarded_to_child_view_when_not_stick)
{
    View view;

    StrictMock<MockView> mock_view;
    view.add_view(&mock_view);

    mock_view.set_position(glm::dvec2(0, 0));
    mock_view.set_size(glm::dvec2(500, 500));

    glm::dvec2 cursor_inside(100, 200);
    glm::dvec2 cursor_outside(-100, -100);

    // The mouse is clicked outside and released inside the child view so we expect the mocked
    // on_mouse_button() to never be called. StrictMock will ensure this doesn't happen
    view.on_mouse_button(cursor_outside, MouseButton::Primary, Action::Press, Modifiers::None);
    view.on_mouse_button(cursor_inside, MouseButton::Primary, Action::Release, Modifiers::None);
}

TEST(View, draw_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, draw()).Times(1);

    View view;
    view.add_view(&mock_view);
    view.draw();
}

TEST(View, on_cursor_move_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, on_cursor_move(0, 0)).Times(1);

    View view;
    view.add_view(&mock_view);
    view.on_cursor_move(0, 0);
}

TEST(View, on_resize_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, on_resize(0, 0)).Times(1);

    View view;
    view.add_view(&mock_view);
    view.on_resize(0, 0);
}

TEST(View, on_key_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, on_key(Key::A, 0, Action::Press, Modifiers::Control)).Times(1);

    View view;
    view.add_view(&mock_view);
    view.on_key(Key::A, 0, Action::Press, Modifiers::Control);
}

TEST(View, on_scroll_call_forwarded_to_child_view)
{
    MockView mock_view;
    mock_view.set_position(glm::vec2(0, 0));
    mock_view.set_size(glm::vec2(500, 500));
    EXPECT_CALL(mock_view, on_scroll(glm::dvec2(100, 200), 10, 20)).Times(1);

    View view;
    view.add_view(&mock_view);
    view.on_scroll(glm::dvec2(100, 200), 10, 20);
}

TEST(View, hitbox_looks_reasonable)
{
    View view;
    view.set_position(glm::dvec2(100, 100));
    view.set_size(glm::dvec2(100, 100));

    auto hitbox = view.get_hitbox();
    ASSERT_EQ(hitbox.tl, glm::dvec2(100, 100));
    ASSERT_EQ(hitbox.br, glm::dvec2(200, 200));
}
