#include "vector.hpp"
#include "utils.hpp"
#include <math.h>
#include <random>

std::default_random_engine engine;
std::uniform_real_distribution<double> unif(0.,1.);

/* Operations */
Vector Vector::operator*(const double &alpha) const {
	return Vector(alpha*x, alpha*y, alpha*z);
}

Vector Vector::operator*(const Vector &v) const {
	return Vector(x*v.x,y*v.y,z*v.z);
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

void Vector::convertCoordinateSystem(Vector u, Vector v, Vector w) {
	x = x*u.x + y*v.x + z*w.x;
	y = x*u.y + y*v.y + z*w.y;
	z = x*u.z + y*v.z + z*w.z;
}

Vector operator*(double alpha, const Vector &v) {
	return Vector(alpha*v.x, alpha*v.y, alpha*v.z);
}

Vector generateUniformRandomVector() {
	double r1 = unif(engine);
	double r2 = unif(engine);
	double t = sqrt(1-r2);
	//std::cerr << cos(2*PI*r1)*t << "," << sin(2*PI*r1)*t << "\n";
	
	return Vector(cos(2*PI*r1)*t, sin(2*PI*r1)*t, sqrt(r2));
}
