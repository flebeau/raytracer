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
	typedef std::pair<double, Sphere> Intersection;
	
	Scene(Light l) : light(l) {}
	
	Scene::Intersection intersect(const Ray &ray) const;
	Vector getColor(const Ray &r, int n);
	
	std::vector<Sphere> spheres;
    Light light;
};


#endif
