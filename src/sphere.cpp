#include <math.h>
#include "sphere.hpp"

double Sphere::intersect(const Ray &ray) const {
	double b = 2. * ray.direction.sp(ray.origin - origin);
	double c = (ray.origin - origin).snorm() - radius * radius;
	
	double delta = b*b - 4.*c;
	if (delta < 0)
		return 0;
	
	double t1 = (-b - sqrt(delta))/2.;
	if (t1 > 0)
		return t1;
	
	double t2 = (-b + sqrt(delta))/2.;
	if (t2 > 0)
		return t2;
	
	return 0;
}
