#ifndef DE_CORE_MATH_HPP
#define DE_CORE_MATH_HPP

#include <engine/core/vector_2d.hpp>

namespace de
{
/// Point `t` of the way from `a` to `b`; 0 is `a`, 1 is `b`.
template <typename T>
constexpr Vector2D<T> lerp(const Vector2D<T>& a, const Vector2D<T>& b, T t)
{
    return a + (b - a) * t;
}

/// `velocity` mirrored off a surface with the given normal:
/// v' = v - 2 * dot(v, n) * n. The normal need not be unit length; it is
/// normalised here.
template <typename T>
Vector2D<T> reflect(const Vector2D<T>& velocity, Vector2D<T> normal)
{
    normal.normalize();
    return velocity - normal * (static_cast<T>(2) * velocity.dot(normal));
}

/// A circle for overlap tests, by centre and radius.
template <typename T>
struct Circle
{
    T x{};
    T y{};
    T radius{};

    constexpr T left() const noexcept { return x - radius; }
    constexpr T right() const noexcept { return x + radius; }
    constexpr T top() const noexcept { return y - radius; }
    constexpr T bottom() const noexcept { return y + radius; }

    /// True when the circles overlap; touching edges do not count.
    constexpr bool intersects(const Circle<T>& other) const noexcept
    {
        const T dx = x - other.x;
        const T dy = y - other.y;
        const T radii = radius + other.radius;
        return (dx * dx + dy * dy) < (radii * radii);
    }
};

} // namespace de

#endif // DE_CORE_MATH_HPP
