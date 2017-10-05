#include <math.h>
#include "sphere.hpp"

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
