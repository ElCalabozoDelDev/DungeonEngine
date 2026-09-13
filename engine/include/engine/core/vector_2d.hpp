#ifndef DE_CORE_VECTOR_2D_HPP
#define DE_CORE_VECTOR_2D_HPP

#include <cmath>

namespace de
{
template <typename T>
class Vector2D
{
private:
    T m_x;
    T m_y;

public:
    constexpr Vector2D() noexcept : m_x(T{}), m_y(T{}) {}
    constexpr Vector2D(T x, T y) noexcept : m_x(x), m_y(y) {}

    constexpr T getX() const noexcept { return m_x; }
    constexpr T getY() const noexcept { return m_y; }

    constexpr void setX(T x) noexcept { m_x = x; }
    constexpr void setY(T y) noexcept { m_y = y; }

    T length() const noexcept
    {
        return static_cast<T>(std::sqrt(m_x * m_x + m_y * m_y));
    }

    constexpr T lengthSquared() const noexcept { return m_x * m_x + m_y * m_y; }

    constexpr T dot(const Vector2D<T>& other) const noexcept
    {
        return m_x * other.m_x + m_y * other.m_y;
    }

    constexpr bool operator==(const Vector2D<T>& other) const noexcept
    {
        return m_x == other.m_x && m_y == other.m_y;
    }

    constexpr Vector2D<T> operator+(const Vector2D<T>& v2) const noexcept
    {
        return Vector2D<T>(m_x + v2.m_x, m_y + v2.m_y);
    }

    friend Vector2D<T>& operator+=(Vector2D<T>& v1, const Vector2D<T>& v2)
    {
        v1.m_x += v2.m_x;
        v1.m_y += v2.m_y;

        return v1;
    }

    constexpr Vector2D<T> operator-(const Vector2D<T>& v2) const noexcept
    {
        return Vector2D<T>(m_x - v2.m_x, m_y - v2.m_y);
    }

    friend Vector2D<T>& operator-=(Vector2D<T>& v1, const Vector2D<T>& v2)
    {
        v1.m_x -= v2.m_x;
        v1.m_y -= v2.m_y;

        return v1;
    }

    constexpr Vector2D<T> operator*(T scalar) const noexcept
    {
        return Vector2D<T>(m_x * scalar, m_y * scalar);
    }

    constexpr Vector2D<T>& operator*=(T scalar) noexcept
    {
        m_x *= scalar;
        m_y *= scalar;

        return *this;
    }

    constexpr Vector2D<T> operator/(T scalar) const noexcept
    {
        return Vector2D<T>(m_x / scalar, m_y / scalar);
    }

    constexpr Vector2D<T>& operator/=(T scalar) noexcept
    {
        m_x /= scalar;
        m_y /= scalar;

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
