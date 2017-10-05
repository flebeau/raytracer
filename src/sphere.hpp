#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "vector.hpp"
#include "ray.hpp"
#include "material.hpp"

class Sphere {
public:
	/* Constructors */
	Sphere(Vector o, double r) : origin(o), radius(r) {}
	Sphere(Vector o, double r, Material m) : origin(o), radius(r), material(m) {}
	
	// Returns 0 if no intersection and t such that C + t.V is the intersection otherwise
	double intersect(const Ray &ray) const;
	
	Vector origin;
	double radius;
	Material material;
};

#endif
