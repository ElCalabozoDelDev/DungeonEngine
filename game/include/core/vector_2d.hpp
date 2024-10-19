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

    constexpr Vector2D<T>(float x, float y) noexcept : m_x(x), m_y(y) {}

    const float getX() const { return m_x; }
    const float getY() const { return m_y; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }

    int length() { return sqrt(m_x * m_x + m_y * m_y); }

    Vector2D<T> operator+(const Vector2D<T>& v2) const
    {
        return Vector2D<T>(m_x + v2.m_x, m_y + v2.m_y);
    }
    friend Vector2D<T>& operator+=(Vector2D<T>& v1, const Vector2D<T>& v2)
    {
        v1.m_x += v2.m_x;
        v1.m_y += v2.m_y;

        return v1;
    }

    Vector2D<T> operator-(const Vector2D<T>& v2) const
    {
        return Vector2D<T>(m_x - v2.m_x, m_y - v2.m_y);
    }
    friend Vector2D<T>& operator-=(Vector2D<T>& v1, const Vector2D<T>& v2)
    {
        v1.m_x -= v2.m_x;
        v1.m_y -= v2.m_y;

        return v1;
    }

    Vector2D<T> operator*(float scalar) const
    {
        return Vector2D<T>(m_x * scalar, m_y * scalar);
    }

    Vector2D<T>& operator*=(float scalar)
    {
        m_x *= scalar;
        m_y *= scalar;

        return *this;
    }

    Vector2D<T> operator/(float scalar) const
    {
        return Vector2D<T>(m_x / scalar, m_y / scalar);
    }

    Vector2D<T>& operator/=(float scalar)
    {
        m_x /= scalar;
        m_y /= scalar;

        return *this;
    }

    void normalize()
    {
        int l = length();
        if (l > 0)
        {
            (*this) *= Q_rsqrt(l);
        }
    }
};

#endif
