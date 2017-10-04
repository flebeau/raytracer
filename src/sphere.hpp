#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "vector.hpp"
#include "ray.hpp"

class Sphere {
public:
	/* Constructors */
	Sphere(Vector o, double r) : origin(o), radius(r) {}
	
	// Returns 0 if no intersection and t such that C + t.V is the intersection otherwise
	double intersect(const Ray &ray) const;
	
	Vector origin;
	double radius;
};

#endif
