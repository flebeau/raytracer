#include "vector.hpp"
#include <math.h>

/* Operations */
Vector Vector::operator*(const double &alpha) const {
	return Vector(alpha*x, alpha*y, alpha*z);
}
	
Vector Vector::operator+(const Vector &v) const {
	return Vector(x+v.x, y+v.y, z+v.z);
}
	
Vector Vector::operator-(const Vector &v) const {
	return Vector(x-v.x, y-v.y, z-v.z);
}
	
Vector Vector::operator-() const {
	return Vector(-x,-y,-z);
}

double Vector::sp(const Vector &v) const {
	return (x*v.x + y*v.y + z*v.z);
}

Vector Vector::vp(const Vector &v) const {
	return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

double Vector::snorm() const {
	return x*x + y*y + z*z;
}

double Vector::norm() const {
	return sqrt(snorm());
}

Vector Vector::normalize() const {
	double n = norm();
	return Vector(x/n, y/n, z/n);
}

Vector operator*(double alpha, const Vector &v) {
	return Vector(alpha*v.x, alpha*v.y, alpha*v.z);
}
