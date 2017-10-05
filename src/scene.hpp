#ifndef SCENE_HPP
#define SCENE_HPP

#include "sphere.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <vector>
#include <utility>

class Light {
public:
	Light(Vector p, double i) : position(p), intensity(i) {}
	
	Vector position;
	double intensity;
};

class Scene {
public:
	struct Intersection {
		double t;
		Sphere sphere;
		bool entrance;
	};
	
	Scene(Light l) : light(l) {}
	
	Intersection intersect(const Ray &ray) const;
	Vector getColor(const Ray &r, int n);
	
	std::vector<Sphere> spheres;
    Light light;
};


#endif
