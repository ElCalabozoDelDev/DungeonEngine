#include <doctest/doctest.h>
#include <engine/core/vector_2d.hpp>

using de::Vector2D;

TEST_CASE("Vector2D arithmetic")
{
    const Vector2D<float> a(3.0f, 4.0f);
    const Vector2D<float> b(1.0f, 2.0f);

    CHECK((a + b).getX() == doctest::Approx(4.0f));
    CHECK((a + b).getY() == doctest::Approx(6.0f));
    CHECK((a - b).getX() == doctest::Approx(2.0f));
    CHECK((a * 2.0f).getY() == doctest::Approx(8.0f));
    CHECK((a / 2.0f).getX() == doctest::Approx(1.5f));
}

TEST_CASE("Vector2D compound assignment mutates")
{
    // operator*= and operator/= used to be declared const while assigning to
    // members. That only compiled because nothing ever instantiated them,
    // which also made normalize() dead broken code.
    Vector2D<float> v(3.0f, 4.0f);
    v *= 2.0f;
    CHECK(v.getX() == doctest::Approx(6.0f));
    v /= 4.0f;
    CHECK(v.getY() == doctest::Approx(2.0f));
}

TEST_CASE("Vector2D length is not truncated")
{
    // length() returned int, so a 3-4-5 triangle was fine but anything
    // fractional was silently floored.
    const Vector2D<float> v(1.0f, 1.0f);
    CHECK(v.length() == doctest::Approx(1.41421356f));
    CHECK(v.lengthSquared() == doctest::Approx(2.0f));
}

TEST_CASE("Vector2D normalize")
{
    Vector2D<float> v(0.0f, 5.0f);
    v.normalize();
    CHECK(v.length() == doctest::Approx(1.0f));
    CHECK(v.getY() == doctest::Approx(1.0f));

    SUBCASE("a zero vector is left alone rather than producing NaN")
    {
        Vector2D<float> zero(0.0f, 0.0f);
        zero.normalize();
        CHECK(zero.getX() == doctest::Approx(0.0f));
        CHECK(zero.getY() == doctest::Approx(0.0f));
    }
}
