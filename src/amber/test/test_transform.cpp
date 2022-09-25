#include <utils/transform.hpp>
#include <gtest/gtest.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/ext/matrix_relational.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace amber;

static constexpr double EPSILON = 10e-5;

template <class T> testing::AssertionResult AreEqual(T expected, T actual)
{
    if (glm::all(glm::equal(expected, actual, EPSILON)))
        return testing::AssertionSuccess();
    else
        return testing::AssertionFailure() << "Expected: " << glm::to_string(expected)
                                           << ", Actual: " << glm::to_string(actual);
}

TEST(Transform, defaultContrstuctor_transformIsIdentity)
{
    const Transform<double> t;
    const glm::dmat3 ident(1.0);
    EXPECT_TRUE(AreEqual(t.matrix(), ident));
}

TEST(Transform, transformTranslatedAndScaled_roundTripReturnsSameVector)
{
    using namespace glm;

    Transform<double> t;
    t.translate(dvec2(0.2, 2.4));
    t.scale(dvec2(3.4, -4.2));

    const dvec2 input(4.3, -9.4);
    const auto output = t.apply(t.apply_inverse(input));
    EXPECT_TRUE(AreEqual(input, output));
}

TEST(Transform, transformTranslatedAndScaled_roundTripUsingRawMatriciesReturnsSameVector)
{
    using namespace glm;

    Transform<double> t;
    t.translate(dvec2(0.2, 2.4));
    t.scale(dvec2(3.4, -4.2));

    const dvec2 input(4.3, -9.4);
    const dvec2 output = t.matrix() * t.matrix_inverse() * dvec3(input, 1.0);
    EXPECT_TRUE(AreEqual(input, output));
}

TEST(Transform, transformRelative)
{
    using namespace glm;

    Transform<double> t;
    t.scale(dvec2(2.0));
    t.translate(dvec2(-0.4, 9.6));

    const dvec2 expected(0.8, -9.0);
    const auto actual = t.apply_relative(dvec2(0.4, -4.5));
    EXPECT_TRUE(AreEqual(expected, actual));
}

TEST(Transform, transformInverseRelative)
{
    using namespace glm;

    Transform<double> t;
    t.scale(dvec2(2.0));
    t.translate(dvec2(-0.4, 9.6));

    const dvec2 expected(0.2, -2.25);
    const auto actual = t.apply_inverse_relative(dvec2(0.4, -4.5));
    EXPECT_TRUE(AreEqual(expected, actual));
}

TEST(Transform, transformApply)
{
    using namespace glm;

    Transform<double> t;
    t.scale(dvec2(2.0, 4.5));
    t.translate(dvec2(0.2, 2.4));

    const auto actual = t.apply(dvec2(3.4, -0.3));
    const dvec2 expected((0.2 + 3.4) * 2.0, (2.4 - 0.3) * 4.5);
    EXPECT_TRUE(AreEqual(expected, actual));
}
