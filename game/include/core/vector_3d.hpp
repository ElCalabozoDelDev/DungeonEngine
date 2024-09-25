#ifndef VECTOR_3D_HPP
#define VECTOR_3D_HPP

#include <cmath>
#include "core/q_rsqrt.hpp"
class Vector3D
{
public:
    Vector3D()
    {
        m_x = 0;
        m_y = 0;
        m_z = 0;
    }
    
    Vector3D(float x, float y, float z): m_x(x), m_y(y), m_z(z) {}
    
    const float getX() const { return m_x; }
    const float getY() const { return m_y; }
    const float getZ() const { return m_z; }
    
    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void setZ(float z) { m_z = z; }
    
    float length() const { return sqrt(m_x * m_x + m_y * m_y + m_z * m_z); }

    Vector3D operator+(const Vector3D& v) const { return Vector3D(m_x + v.m_x, m_y + v.m_y, m_z + v.m_z); }
    friend Vector3D& operator+=(Vector3D& v1, const Vector3D& v2)
    {
        v1.m_x += v2.m_x;
        v1.m_y += v2.m_y;
        v1.m_z += v2.m_z;
        
        return v1;
    }
    
    Vector3D operator-(const Vector3D& v) const { return Vector3D(m_x - v.m_x, m_y - v.m_y, m_z - v.m_z); }
    friend Vector3D& operator-=(Vector3D& v1, const Vector3D& v2)
    {
        v1.m_x -= v2.m_x;
        v1.m_y -= v2.m_y;
        v1.m_z -= v2.m_z;
        
        return v1;
    }
    
    Vector3D operator*(float scalar) const { return Vector3D(m_x * scalar, m_y * scalar, m_z * scalar); }
    
    Vector3D& operator*=(float scalar)
    {
        m_x *= scalar;
        m_y *= scalar;
        m_z *= scalar;
        
        return *this;
    }
    
    Vector3D operator/(float scalar) const { return Vector3D(m_x / scalar, m_y / scalar, m_z / scalar); }
    
    Vector3D& operator/=(float scalar)
    {
        m_x /= scalar;
        m_y /= scalar;
        m_z /= scalar;
        
        return *this;
    }
    
    void normalize()
    {
        float l = length();
        if (l > 0)
        {
            (*this) *= Q_rsqrt(l);  // Usar la raíz cuadrada inversa en lugar de 1/l
        }
    }

    
    float m_x;
    float m_y;
    float m_z;
};

#endif
