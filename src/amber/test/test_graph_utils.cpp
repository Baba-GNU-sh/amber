#include <graph_utils.hpp>
#include <gtest/gtest.h>

TEST(GraphUtils, hit_test)
{
    ASSERT_TRUE(GraphUtils::hit_test(glm::vec2(0.5, 0.5), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)));
    ASSERT_TRUE(GraphUtils::hit_test(glm::vec2(1.0, 1.0), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)));

    ASSERT_FALSE(GraphUtils::hit_test(glm::vec2(1.5, 0.5), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)));
    ASSERT_FALSE(GraphUtils::hit_test(glm::vec2(0.5, 1.5), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)));
    ASSERT_FALSE(GraphUtils::hit_test(glm::vec2(1.5, 1.5), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0)));
}
