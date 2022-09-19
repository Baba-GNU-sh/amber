#include "view.hpp"
#include <glm/ext/vector_double2.hpp>
#include <glm/ext/vector_float2.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace glm;
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
    MOCK_METHOD(void, on_mouse_button, (const dvec2 &, MouseButton, Action, Modifiers), (override));
    MOCK_METHOD(void, on_scroll, (const dvec2 &, double, double), (override));
    MOCK_METHOD(void, draw, (), (override));
    MOCK_METHOD(void, on_cursor_move, (double x, double y), (override));
    MOCK_METHOD(void, on_resize, (int, int), (override));
    MOCK_METHOD(void, on_key, (Key, int, Action action, Modifiers mods), (override));
};

TEST(View, mouseClickedWithinChildView_childReceivesPressAndReleaseEvents)
{
    MockView mock_view;
    mock_view.set_position(dvec2(0, 0));
    mock_view.set_size(dvec2(500, 500));

    View view;
    view.add_view(&mock_view);

    EXPECT_CALL(
        mock_view,
        on_mouse_button(dvec2(100, 100), MouseButton::Primary, Action::Press, Modifiers::None));
    view.on_mouse_button(dvec2(100, 100), MouseButton::Primary, Action::Press, Modifiers::None);

    EXPECT_CALL(
        mock_view,
        on_mouse_button(dvec2(100, 100), MouseButton::Primary, Action::Release, Modifiers::None));
    view.on_mouse_button(dvec2(100, 100), MouseButton::Primary, Action::Release, Modifiers::None);
}

TEST(View, mousePressedWithinChildViewButReleasedOutside_checkChildReceivesPressAndReleaseEvents)
{
    MockView mock_view;
    mock_view.set_position(dvec2(0, 0));
    mock_view.set_size(dvec2(500, 500));

    View view;
    view.add_view(&mock_view);

    EXPECT_CALL(
        mock_view,
        on_mouse_button(dvec2(100, 100), MouseButton::Primary, Action::Press, Modifiers::None));
    view.on_mouse_button(dvec2(100, 100), MouseButton::Primary, Action::Press, Modifiers::None);

    EXPECT_CALL(
        mock_view,
        on_mouse_button(dvec2(800, 800), MouseButton::Primary, Action::Release, Modifiers::None));
    view.on_mouse_button(dvec2(800, 800), MouseButton::Primary, Action::Release, Modifiers::None);
}

TEST(View, mouseClickedOusideChildView_checkChildReceivesNoEvents)
{
    StrictMock<MockView> mock_view;
    mock_view.set_position(dvec2(0, 0));
    mock_view.set_size(dvec2(500, 500));

    View view;
    view.add_view(&mock_view);

    view.on_mouse_button(dvec2(800, 800), MouseButton::Primary, Action::Press, Modifiers::None);
    view.on_mouse_button(dvec2(800, 800), MouseButton::Primary, Action::Release, Modifiers::None);
}

TEST(View, mousePressedOusideChildViewButReleasedInside_checkChildReceivesNoEvents)
{
    StrictMock<MockView> mock_view;
    mock_view.set_position(dvec2(0, 0));
    mock_view.set_size(dvec2(500, 500));

    View view;
    view.add_view(&mock_view);

    view.on_mouse_button(dvec2(800, 800), MouseButton::Primary, Action::Press, Modifiers::None);
    view.on_mouse_button(dvec2(100, 100), MouseButton::Primary, Action::Release, Modifiers::None);
}

TEST(View, draw_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, draw());

    View view;
    view.add_view(&mock_view);
    view.draw();
}

TEST(View, on_cursor_move_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, on_cursor_move(0, 0));

    View view;
    view.add_view(&mock_view);
    view.on_cursor_move(0, 0);
}

TEST(View, on_resize_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, on_resize(0, 0));

    View view;
    view.add_view(&mock_view);
    view.on_resize(0, 0);
}

TEST(View, on_key_call_forwarded_to_child_view)
{
    MockView mock_view;
    EXPECT_CALL(mock_view, on_key(Key::A, 0, Action::Press, Modifiers::Control));

    View view;
    view.add_view(&mock_view);
    view.on_key(Key::A, 0, Action::Press, Modifiers::Control);
}

TEST(View, on_scroll_call_forwarded_to_child_view)
{
    MockView mock_view;
    mock_view.set_position(vec2(0, 0));
    mock_view.set_size(vec2(500, 500));
    EXPECT_CALL(mock_view, on_scroll(dvec2(100, 200), 10, 20));

    View view;
    view.add_view(&mock_view);
    view.on_scroll(dvec2(100, 200), 10, 20);
}

TEST(View, hitbox_looks_reasonable)
{
    View view;
    view.set_position(dvec2(100, 100));
    view.set_size(dvec2(100, 100));

    auto hitbox = view.get_hitbox();
    ASSERT_EQ(hitbox.tl, dvec2(100, 100));
    ASSERT_EQ(hitbox.br, dvec2(200, 200));
}
