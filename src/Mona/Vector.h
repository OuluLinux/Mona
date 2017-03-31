#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

#include "Common.h"

//------------------------------------------------------------------------//
// Misc. Constants
//------------------------------------------------------------------------//

float const pi  = 3.14159265f;
float const tol = 0.0000000001f;                  // float type tolerance

//------------------------------------------------------------------------//
// Misc. Functions
//------------------------------------------------------------------------//
inline float DegreesToRadians(float deg)
{
   return (deg * pi / 180.0f);
}


inline float RadiansToDegrees(float rad)
{
   return (rad * 180.0f / pi);
}


//------------------------------------------------------------------------//
// Vector3f Class and vector functions
//------------------------------------------------------------------------//
class Vector3f
{
public:
   float x;
   float y;
   float z;

   Vector3f(void);
   Vector3f(float xi, float yi, float zi);

   float Magnitude(void);
   void Normalize(void);
   void Normalize(float);
   void Reverse(void);

   float Distance(Vector3f);
   float SquareDistance(Vector3f);
   void Zero(void);

   Vector3f& operator+=(Vector3f u);                  // vector addition
   Vector3f& operator-=(Vector3f u);                  // vector subtraction
   Vector3f& operator*=(float s);                   // scalar multiply
   Vector3f& operator/=(float s);                   // scalar divide

   Vector3f operator-(void);
};

inline Vector3f operator+(Vector3f u, Vector3f v);
inline Vector3f operator-(Vector3f u, Vector3f v);
inline Vector3f operator^(Vector3f u, Vector3f v);
inline float operator *(Vector3f u, Vector3f v);
inline Vector3f operator *(float s, Vector3f u);
inline Vector3f operator *(Vector3f u, float s);
inline Vector3f operator/(Vector3f u, float s);
inline float TripleScalarProduct(Vector3f u, Vector3f v, Vector3f w);

inline Vector3f::Vector3f(void)
{
   x = 0;
   y = 0;
   z = 0;
}


inline Vector3f::Vector3f(float xi, float yi, float zi)
{
   x = xi;
   y = yi;
   z = zi;
}


inline float Vector3f::Magnitude(void)
{
   return ((float)sqrt(x * x + y * y + z * z));
}


inline void Vector3f::Normalize(void)
{
   float m = (float)sqrt(x * x + y * y + z * z);

   if (m <= tol)
   {
      m = 1;
   }
   x /= m;
   y /= m;
   z /= m;

   if (fabs(x) < tol)
   {
      x = 0.0f;
   }
   if (fabs(y) < tol)
   {
      y = 0.0f;
   }
   if (fabs(z) < tol)
   {
      z = 0.0f;
   }
}


inline void Vector3f::Normalize(float n)
{
   float m = (float)sqrt(x * x + y * y + z * z);

   if (m <= tol)
   {
      m = 1;
   }
   m  = n / m;
   x *= m;
   y *= m;
   z *= m;

   if (fabs(x) < tol)
   {
      x = 0.0f;
   }
   if (fabs(y) < tol)
   {
      y = 0.0f;
   }
   if (fabs(z) < tol)
   {
      z = 0.0f;
   }
}


inline void Vector3f::Reverse(void)
{
   x = -x;
   y = -y;
   z = -z;
}


inline float Vector3f::Distance(Vector3f v)
{
   float dx, dy, dz;

   dx = x - v.x;
   dy = y - v.y;
   dz = z - v.z;
   return ((float)sqrt(dx * dx + dy * dy + dz * dz));
}


inline float Vector3f::SquareDistance(Vector3f v)
{
   float dx, dy, dz;

   dx = x - v.x;
   dy = y - v.y;
   dz = z - v.z;
   return ((float)(dx * dx + dy * dy + dz * dz));
}


inline void Vector3f::Zero(void)
{
   x = 0.0;
   y = 0.0;
   z = 0.0;
}


inline Vector3f& Vector3f::operator+=(Vector3f u)
{
   x += u.x;
   y += u.y;
   z += u.z;
   return (*this);
}


inline Vector3f& Vector3f::operator-=(Vector3f u)
{
   x -= u.x;
   y -= u.y;
   z -= u.z;
   return (*this);
}


inline Vector3f& Vector3f::operator*=(float s)
{
   x *= s;
   y *= s;
   z *= s;
   return (*this);
}


inline Vector3f& Vector3f::operator/=(float s)
{
   x /= s;
   y /= s;
   z /= s;
   return (*this);
}


inline Vector3f Vector3f::operator-(void)
{
   return (Vector3f(-x, -y, -z));
}


inline Vector3f operator+(Vector3f u, Vector3f v)
{
   return (Vector3f(u.x + v.x, u.y + v.y, u.z + v.z));
}


inline Vector3f operator-(Vector3f u, Vector3f v)
{
   return (Vector3f(u.x - v.x, u.y - v.y, u.z - v.z));
}


// Vector3f cross product (u cross v)
inline Vector3f operator^(Vector3f u, Vector3f v)
{
   return (Vector3f(u.y * v.z - u.z * v.y,
                 -u.x * v.z + u.z * v.x,
                 u.x * v.y - u.y * v.x));
}


// Vector3f dot product
inline float operator *(Vector3f u, Vector3f v)
{
   return (u.x * v.x + u.y * v.y + u.z * v.z);
}


inline Vector3f operator *(float s, Vector3f u)
{
   return (Vector3f(u.x * s, u.y * s, u.z * s));
}


inline Vector3f operator *(Vector3f u, float s)
{
   return (Vector3f(u.x * s, u.y * s, u.z * s));
}


inline Vector3f operator/(Vector3f u, float s)
{
   return (Vector3f(u.x / s, u.y / s, u.z / s));
}


// triple scalar product (u dot (v cross w))
inline float TripleScalarProduct(Vector3f u, Vector3f v, Vector3f w)
{
   return (float((u.x * (v.y * w.z - v.z * w.y)) +
                (u.y * (-v.x * w.z + v.z * w.x)) +
                (u.z * (v.x * w.y - v.y * w.x))));
   //return u*(v^w);
}


#endif
