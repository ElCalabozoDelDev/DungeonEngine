#ifndef DE_CORE_VECTOR_2D_HPP
#define DE_CORE_VECTOR_2D_HPP

#include <cmath>

namespace de
{
/// A 2D vector: `x` and `y`, public.
///
/// An aggregate rather than a class with getters and setters, which bought
/// nothing for two numbers and turned every `p.x += d` into
/// `p.setX(p.getX() + d)`. Construct it as `Vector2D<float>(x, y)` or
/// `Vector2D<float>{x, y}`.
template <typename T>
struct Vector2D
{
    T x{};
    T y{};

    T length() const noexcept
    {
        return static_cast<T>(std::sqrt(x * x + y * y));
    }

    constexpr T lengthSquared() const noexcept { return x * x + y * y; }

    constexpr T dot(const Vector2D<T>& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    constexpr bool operator==(const Vector2D<T>& other) const noexcept
    {
        return x == other.x && y == other.y;
    }

    constexpr Vector2D<T> operator+(const Vector2D<T>& other) const noexcept
    {
        return Vector2D<T>{x + other.x, y + other.y};
    }

    constexpr Vector2D<T>& operator+=(const Vector2D<T>& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vector2D<T> operator-(const Vector2D<T>& other) const noexcept
    {
        return Vector2D<T>{x - other.x, y - other.y};
    }

    constexpr Vector2D<T>& operator-=(const Vector2D<T>& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vector2D<T> operator*(T scalar) const noexcept
    {
        return Vector2D<T>{x * scalar, y * scalar};
    }

    constexpr Vector2D<T>& operator*=(T scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vector2D<T> operator/(T scalar) const noexcept
    {
        return Vector2D<T>{x / scalar, y / scalar};
    }

    constexpr Vector2D<T>& operator/=(T scalar) noexcept
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    /// Scales the vector to unit length. A zero-length vector is left alone.
    void normalize() noexcept
    {
        const T l = length();
        if (l > T{})
        {
            *this /= l;
        }
    }
};

} // namespace de

#endif // DE_CORE_VECTOR_2D_HPP
