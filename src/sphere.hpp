#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "vector.hpp"
#include "ray.hpp"
#include "material.hpp"
#include <utility>

class Sphere {
public:
	/* Constructors */
	Sphere() : origin(), radius(0) {}
	Sphere(Vector o, double r) : origin(o), radius(r) {}
	Sphere(Vector o, double r, Material m) : origin(o), radius(r), material(m) {}
	
	typedef std::pair<double, bool> Intersection;
	
	static bool compareByRadius(const Sphere &s1, const Sphere &s2);
	
	// Returns 0 if no intersection and t such that C + t.V is the intersection otherwise.
	// If t > 0 then returns also true if the ray is entering the sphere
	Intersection intersect(const Ray &ray) const;
	
	Vector origin;
	double radius;
	Material material;
};

#endif
