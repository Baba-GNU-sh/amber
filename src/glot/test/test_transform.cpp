#include <utils/transform.hpp>
#include <gtest/gtest.h>
#include <glm/gtx/matrix_transform_2d.hpp>

static constexpr double EPSILON = 10e-5;

static bool compare_matricies(const glm::dmat3 &a, const glm::dmat3 &b)
{
    glm::dvec2 input(0.0);
    glm::dvec2 result = a * (glm::inverse(b) * glm::dvec3(0.0, 0.0, 1.0));
    const auto delta = input - result;
    return glm::all(glm::lessThan(delta, glm::dvec2(EPSILON)));
}

TEST(Transform, initNoInputsMakesIdentTransform)
{
    Transform<double> a;
    glm::dmat3 ident(1.0);
    ASSERT_TRUE(compare_matricies(a.matrix(), ident));
}

TEST(Transform, roundTripTest)
{
    glm::dmat3 m(1.0);
    m = glm::translate(m, glm::dvec2(0.2, 2.4));
    m = glm::scale(m, glm::dvec2(3.4, -0.3));
    Transform<double> t(m);
    ASSERT_TRUE(compare_matricies(t.matrix(), t.matrix_inverse()));
}

TEST(Transform, roundTripTestWithRawMatricies)
{
    glm::dmat3 m(1.0);
    m = glm::translate(m, glm::dvec2(0.2, 2.4));
    m = glm::scale(m, glm::dvec2(3.4, -0.3));
    Transform<double> t(m);

    glm::dvec2 input(0.0);
    glm::dvec2 result = t.matrix_inverse() * (t.matrix() * glm::dvec3(0.0, 0.0, 1.0));
    auto delta = glm::abs(result - input);
    double EPSILON = 10e-5;
    ASSERT_TRUE(glm::all(glm::lessThan(delta, glm::dvec2(EPSILON))));
}

TEST(Transform, transformedVectorIsScaledAndTranslated)
{
    glm::dmat3 m(1.0);
    m = glm::translate(m, glm::dvec2(0.2, 2.4));
    m = glm::scale(m, glm::dvec2(3.4, -0.3));
    Transform<double> t(m);

    glm::dvec2 input(0.0);
    auto result = t.apply(input);
    ASSERT_DOUBLE_EQ(result.x, 0.2);
    ASSERT_DOUBLE_EQ(result.y, 2.4);

    glm::dvec2 input2(2.0);
    result = t.apply(input2);
    ASSERT_DOUBLE_EQ(result.x, 0.2 + 3.4 * 2);
    ASSERT_DOUBLE_EQ(result.y, 2.4 - 0.3 * 2);
}
