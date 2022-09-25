#include <gtest/gtest.h>
#include "utils/hitbox.hpp"

using namespace glm;
using namespace amber;

TEST(Hitbox, hit_test)
{
    Hitbox<double> hitbox{dvec2(0.0, 0.0), dvec2(1.0, 1.0)};

    ASSERT_TRUE(hitbox.test(dvec2(0.5, 0.5)));
    ASSERT_TRUE(hitbox.test(dvec2(1.0, 1.0)));

    ASSERT_FALSE(hitbox.test(dvec2(1.5, 0.5)));
    ASSERT_FALSE(hitbox.test(dvec2(0.5, 1.5)));
    ASSERT_FALSE(hitbox.test(dvec2(1.5, 1.5)));

    ASSERT_FALSE(hitbox.test(dvec2(-1, 0.5)));
    ASSERT_FALSE(hitbox.test(dvec2(0.5, -1)));
    ASSERT_FALSE(hitbox.test(dvec2(-1, -1)));
}

TEST(Hitbox, hit_test_doubles)
{
    Hitbox<double> hitbox{dvec2(0.0, 0.0), dvec2(1.0, 1.0)};

    ASSERT_TRUE(hitbox.test(0.5, 0.5));
    ASSERT_TRUE(hitbox.test(1.0, 1.0));

    ASSERT_FALSE(hitbox.test(1.5, 0.5));
    ASSERT_FALSE(hitbox.test(0.5, 1.5));
    ASSERT_FALSE(hitbox.test(1.5, 1.5));

    ASSERT_FALSE(hitbox.test(-1, 0.5));
    ASSERT_FALSE(hitbox.test(0.5, -1));
    ASSERT_FALSE(hitbox.test(-1, -1));
}
