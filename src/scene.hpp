#ifndef SCENE_HPP
#define SCENE_HPP

#include "sphere.hpp"
#include "ray.hpp"
#include <vector>
#include <utility>

class Scene {
public:
	typedef std::pair<double, Sphere> Intersection;
	
	Scene::Intersection intersect(const Ray &ray) const;
	
	std::vector<Sphere> spheres;
};


#endif
