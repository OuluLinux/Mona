#ifndef _Mona_Vector_h_
#define _Mona_Vector_h_

#include "Common.h"

//------------------------------------------------------------------------//
// Misc. Constants
//------------------------------------------------------------------------//

double const pi  = 3.14159265f;
double const tol = 0.0000000001f;                  // double type tolerance

//------------------------------------------------------------------------//
// Misc. Functions
//------------------------------------------------------------------------//
inline double DegreesToRadians(double deg) {
	return (deg * pi / 180.0f);
}


inline double RadiansToDegrees(double rad) {
	return (rad * 180.0f / pi);
}


//------------------------------------------------------------------------//
// Vector3f Class and vector functions
//------------------------------------------------------------------------//
class Vector3f {
public:
	double x;
	double y;
	double z;

	Vector3f(void);
	Vector3f(double xi, double yi, double zi);

	double Magnitude(void);
	void Normalize(void);
	void Normalize(double);
	void Reverse(void);

	double Distance(Vector3f);
	double SquareDistance(Vector3f);
	void Zero(void);

	Vector3f& operator+=(Vector3f u);                  // vector addition
	Vector3f& operator-=(Vector3f u);                  // vector subtraction
	Vector3f& operator*=(double s);                   // scalar multiply
	Vector3f& operator/=(double s);                   // scalar divide

	Vector3f operator-(void);
};

inline Vector3f operator+(Vector3f u, Vector3f v);
inline Vector3f operator-(Vector3f u, Vector3f v);
inline Vector3f operator^(Vector3f u, Vector3f v);
inline double operator *(Vector3f u, Vector3f v);
inline Vector3f operator *(double s, Vector3f u);
inline Vector3f operator *(Vector3f u, double s);
inline Vector3f operator/(Vector3f u, double s);
inline double TripleScalarProduct(Vector3f u, Vector3f v, Vector3f w);

inline Vector3f::Vector3f(void) {
	x = 0;
	y = 0;
	z = 0;
}


inline Vector3f::Vector3f(double xi, double yi, double zi) {
	x = xi;
	y = yi;
	z = zi;
}


inline double Vector3f::Magnitude(void) {
	return ((float)sqrt(x * x + y * y + z * z));
}


inline void Vector3f::Normalize(void) {
	double m = (float)sqrt(x * x + y * y + z * z);

	if (m <= tol)
		m = 1;

	x /= m;
	y /= m;
	z /= m;

	if (fabs(x) < tol)
		x = 0.0;

	if (fabs(y) < tol)
		y = 0.0;

	if (fabs(z) < tol)
		z = 0.0;
}


inline void Vector3f::Normalize(double n) {
	double m = (float)sqrt(x * x + y * y + z * z);

	if (m <= tol)
		m = 1;

	m  = n / m;
	x *= m;
	y *= m;
	z *= m;

	if (fabs(x) < tol)
		x = 0.0;

	if (fabs(y) < tol)
		y = 0.0;

	if (fabs(z) < tol)
		z = 0.0;
}


inline void Vector3f::Reverse(void) {
	x = -x;
	y = -y;
	z = -z;
}


inline double Vector3f::Distance(Vector3f v) {
	double dx, dy, dz;
	dx = x - v.x;
	dy = y - v.y;
	dz = z - v.z;
	return ((float)sqrt(dx * dx + dy * dy + dz * dz));
}


inline double Vector3f::SquareDistance(Vector3f v) {
	double dx, dy, dz;
	dx = x - v.x;
	dy = y - v.y;
	dz = z - v.z;
	return ((float)(dx * dx + dy * dy + dz * dz));
}


inline void Vector3f::Zero(void) {
	x = 0.0;
	y = 0.0;
	z = 0.0;
}


inline Vector3f& Vector3f::operator+=(Vector3f u) {
	x += u.x;
	y += u.y;
	z += u.z;
	return (*this);
}


inline Vector3f& Vector3f::operator-=(Vector3f u) {
	x -= u.x;
	y -= u.y;
	z -= u.z;
	return (*this);
}


inline Vector3f& Vector3f::operator*=(double s) {
	x *= s;
	y *= s;
	z *= s;
	return (*this);
}


inline Vector3f& Vector3f::operator/=(double s) {
	x /= s;
	y /= s;
	z /= s;
	return (*this);
}


inline Vector3f Vector3f::operator-(void) {
	return (Vector3f(-x, -y, -z));
}


inline Vector3f operator+(Vector3f u, Vector3f v) {
	return (Vector3f(u.x + v.x, u.y + v.y, u.z + v.z));
}


inline Vector3f operator-(Vector3f u, Vector3f v) {
	return (Vector3f(u.x - v.x, u.y - v.y, u.z - v.z));
}


// Vector3f cross product (u cross v)
inline Vector3f operator^(Vector3f u, Vector3f v) {
	return (Vector3f(u.y * v.z - u.z * v.y,
					 -u.x * v.z + u.z * v.x,
					 u.x * v.y - u.y * v.x));
}


// Vector3f dot product
inline double operator *(Vector3f u, Vector3f v) {
	return (u.x * v.x + u.y * v.y + u.z * v.z);
}


inline Vector3f operator *(double s, Vector3f u) {
	return (Vector3f(u.x * s, u.y * s, u.z * s));
}


inline Vector3f operator *(Vector3f u, double s) {
	return (Vector3f(u.x * s, u.y * s, u.z * s));
}


inline Vector3f operator/(Vector3f u, double s) {
	return (Vector3f(u.x / s, u.y / s, u.z / s));
}


// triple scalar product (u dot (v cross w))
inline double TripleScalarProduct(Vector3f u, Vector3f v, Vector3f w) {
	return (float((u.x * (v.y * w.z - v.z * w.y)) +
				  (u.y * (-v.x * w.z + v.z * w.x)) +
				  (u.z * (v.x * w.y - v.y * w.x))));
	//return u*(v^w);
}


#endif
