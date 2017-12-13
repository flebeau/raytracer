#include <math.h>
#include "sphere.hpp"

bool Sphere::compareByRadius(const std::unique_ptr<Sphere> &s1, const std::unique_ptr<Sphere> &s2) {
	return (s1->radius > s2->radius);
}

Sphere::Intersection Sphere::intersect(const Ray &ray) const {
	double b = 2. * ray.direction.sp(ray.origin - origin);
	double c = (ray.origin - origin).snorm() - radius * radius;
	
	double delta = b*b - 4.*c;
	if (delta < 0)
		return Sphere::Intersection(0, false);
	
	double t1 = (-b - sqrt(delta))/2.;
	if (t1 > 0)
		return Sphere::Intersection(t1, true);
	
	double t2 = (-b + sqrt(delta))/2.;
	if (t2 > 0)
		return Sphere::Intersection(t2, false);
	
	return Sphere::Intersection(0, false);
}

Vector Sphere::color(Vector P) const {
	return material.color;
}

Vector MultiColorSphere::color(Vector P) const {
	double r=0,g=0,b=0;
	if (origin.x - P.x > 0.)
		r = 1;
	if (origin.y - P.y > 0.)
		g = 1;
	if (origin.z - P.z > 0.)
		b = 1;
	return Vector(r,g,b);
}
