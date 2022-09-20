#include <view.hpp>
#include <gtest/gtest.h>

using namespace glm;

TEST(Hitbox, hit_test)
{
    Hitbox hitbox{glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)};

    ASSERT_TRUE(hitbox.test(glm::vec2(0.5, 0.5)));
    ASSERT_TRUE(hitbox.test(glm::vec2(1.0, 1.0)));

    ASSERT_FALSE(hitbox.test(glm::vec2(1.5, 0.5)));
    ASSERT_FALSE(hitbox.test(glm::vec2(0.5, 1.5)));
    ASSERT_FALSE(hitbox.test(glm::vec2(1.5, 1.5)));
}

TEST(Hitbox, hit_test_doubles)
{
    Hitbox hitbox{glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)};

    ASSERT_TRUE(hitbox.test(0.5, 0.5));
    ASSERT_TRUE(hitbox.test(1.0, 1.0));

    ASSERT_FALSE(hitbox.test(1.5, 0.5));
    ASSERT_FALSE(hitbox.test(0.5, 1.5));
    ASSERT_FALSE(hitbox.test(1.5, 1.5));
}
