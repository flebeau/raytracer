#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "vector.hpp"
#include "ray.hpp"
#include "material.hpp"
#include <utility>
#include <memory>

class Sphere {
public:
	/* Constructors */
	Sphere() : origin(), radius(0) {}
	Sphere(Vector o, double r) : origin(o), radius(r) {}
	Sphere(Vector o, double r, Material m) : origin(o), radius(r), material(m) {}
	
	typedef std::pair<double, bool> Intersection;
	
	static bool compareByRadius(const std::unique_ptr<Sphere> &s1, const std::unique_ptr<Sphere> &s2);
	
	// Returns 0 if no intersection and t such that C + t.V is the intersection otherwise.
	// If t > 0 then returns also true if the ray is entering the sphere
	Intersection intersect(const Ray &ray) const;

	virtual Vector color(Vector P) const; // Return the color of a point on the sphere, by default the color of the material
	
	Vector origin;
	double radius;
	Material material;
};

// Example of other type of sphere for which the color is not uniform on the sphere
class MultiColorSphere : public Sphere {
public:
	MultiColorSphere(Vector o, double r) : Sphere(o, r) {}
	
	virtual Vector color(Vector P) const;
};

#endif
