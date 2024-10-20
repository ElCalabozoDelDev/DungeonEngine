#ifndef VECTOR_2D_HPP
#define VECTOR_2D_HPP

#include "core/q_rsqrt.hpp"
#include <math.h>

template<typename T>
class Vector2D
{

private:
    T m_x;
    T m_y;
public:

    constexpr Vector2D<T>(T x, T y) noexcept : m_x(x), m_y(y) {}

    constexpr T getX() const noexcept { return m_x; }
    constexpr T getY() const noexcept { return m_y; }

    constexpr void setX(T x) noexcept { m_x = x; }
    constexpr void setY(T y) noexcept { m_y = y; }

    constexpr int length() const noexcept { return sqrt(m_x * m_x + m_y * m_y); }

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

    constexpr Vector2D<T>& operator*=(T scalar) const noexcept
    {
        m_x *= scalar;
        m_y *= scalar;

        return *this;
    }

    constexpr Vector2D<T> operator/(T scalar) const noexcept
    {
        return Vector2D<T>(m_x / scalar, m_y / scalar);
    }

    constexpr Vector2D<T>& operator/=(T scalar) const noexcept
    {
        m_x /= scalar;
        m_y /= scalar;

        return *this;
    }

    constexpr void normalize() noexcept
    {
        int l = length();
        if (l > 0)
        {
            (*this) *= Q_rsqrt(l);
        }
    }
};

#endif
