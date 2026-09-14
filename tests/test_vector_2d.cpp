#include <doctest/doctest.h>
#include <engine/core/math.hpp>
#include <engine/core/vector_2d.hpp>
#include <engine/spatial/quadtree.hpp>
#include <type_traits>

using de::Vector2D;

TEST_CASE("Vector2D arithmetic")
{
    const Vector2D<float> a(3.0f, 4.0f);
    const Vector2D<float> b(1.0f, 2.0f);

    CHECK((a + b).x == doctest::Approx(4.0f));
    CHECK((a + b).y == doctest::Approx(6.0f));
    CHECK((a - b).x == doctest::Approx(2.0f));
    CHECK((a * 2.0f).y == doctest::Approx(8.0f));
    CHECK((a / 2.0f).x == doctest::Approx(1.5f));
}

TEST_CASE("Vector2D compound assignment mutates")
{
    // operator*= and operator/= used to be declared const while assigning to
    // members. That only compiled because nothing ever instantiated them,
    // which also made normalize() dead broken code.
    Vector2D<float> v(3.0f, 4.0f);
    v *= 2.0f;
    CHECK(v.x == doctest::Approx(6.0f));
    v /= 4.0f;
    CHECK(v.y == doctest::Approx(2.0f));
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
    CHECK(v.y == doctest::Approx(1.0f));

    SUBCASE("a zero vector is left alone rather than producing NaN")
    {
        Vector2D<float> zero(0.0f, 0.0f);
        zero.normalize();
        CHECK(zero.x == doctest::Approx(0.0f));
        CHECK(zero.y == doctest::Approx(0.0f));
    }
}

TEST_CASE("Vector2D dot and equality")
{
    const Vector2D<float> right(1.0f, 0.0f);
    const Vector2D<float> up(0.0f, -1.0f);
    CHECK(right.dot(up) == doctest::Approx(0.0f));
    CHECK(right.dot(right * -1.0f) == doctest::Approx(-1.0f));
    CHECK(Vector2D<float>(3.0f, 4.0f).dot(Vector2D<float>(2.0f, 1.0f)) ==
          doctest::Approx(10.0f));

    CHECK(right == Vector2D<float>(1.0f, 0.0f));
    CHECK_FALSE(right == up);
}

TEST_CASE("lerp walks from one point to the other")
{
    const Vector2D<float> a(10.0f, 20.0f);
    const Vector2D<float> b(30.0f, 60.0f);
    CHECK(de::lerp(a, b, 0.0f) == a);
    CHECK(de::lerp(a, b, 1.0f) == b);
    CHECK(de::lerp(a, b, 0.25f).x == doctest::Approx(15.0f));
    CHECK(de::lerp(a, b, 0.25f).y == doctest::Approx(30.0f));
}

TEST_CASE("reflect mirrors a velocity off a surface")
{
    // Hitting a left wall (normal pointing right) flips x and keeps y.
    const auto bounced =
        de::reflect(Vector2D<float>(-3.0f, 2.0f), Vector2D<float>(1.0f, 0.0f));
    CHECK(bounced.x == doctest::Approx(3.0f));
    CHECK(bounced.y == doctest::Approx(2.0f));

    SUBCASE("with a normal that is not unit length")
    {
        const auto same = de::reflect(Vector2D<float>(-3.0f, 2.0f),
                                      Vector2D<float>(5.0f, 0.0f));
        CHECK(same.x == doctest::Approx(3.0f));
    }

    SUBCASE("off a corner, both axes flip")
    {
        const auto corner = de::reflect(Vector2D<float>(-1.0f, -1.0f),
                                        Vector2D<float>(1.0f, 1.0f));
        CHECK(corner.x == doctest::Approx(1.0f));
        CHECK(corner.y == doctest::Approx(1.0f));
    }
}

TEST_CASE("circles intersect only when they overlap")
{
    const de::Circle<float> a{0.0f, 0.0f, 5.0f};
    CHECK(a.intersects(de::Circle<float>{8.0f, 0.0f, 4.0f}));
    CHECK_FALSE(a.intersects(de::Circle<float>{9.0f, 0.0f, 4.0f})); // touching
    CHECK_FALSE(a.intersects(de::Circle<float>{20.0f, 0.0f, 4.0f}));
    CHECK(a.left() == doctest::Approx(-5.0f));
    CHECK(a.bottom() == doctest::Approx(5.0f));
}

TEST_CASE("Vector2D and Box are plain aggregates")
{
    // Public members instead of getters and setters: p.x += d, not
    // p.setX(p.getX() + d).
    static_assert(std::is_aggregate_v<Vector2D<float>>);
    static_assert(std::is_aggregate_v<de::Box<float>>);

    de::Vector2D<float> p(1.0f, 2.0f);
    p.x += 3.0f;
    CHECK(p == de::Vector2D<float>{4.0f, 2.0f});

    const de::Box<float> box(10.0f, 20.0f, 30.0f, 40.0f);
    CHECK(box.left() == doctest::Approx(10.0f));
    CHECK(box.right() == doctest::Approx(40.0f));
    CHECK(box.bottom() == doctest::Approx(60.0f));
    CHECK(box.contains(de::Box<float>(15.0f, 25.0f, 5.0f, 5.0f)));
    CHECK_FALSE(box.intersects(de::Box<float>(40.0f, 20.0f, 5.0f, 5.0f)));
}
