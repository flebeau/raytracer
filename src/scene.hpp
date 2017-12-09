#ifndef SCENE_HPP
#define SCENE_HPP

#include "sphere.hpp"
#include "ray.hpp"
#include "vector.hpp"
#include <vector>
#include <memory>
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
		int sphere;
		bool entrance;
	};
	
	Scene() : light(Vector(0,0,0),0) {}
	Scene(Light l) : light(l) {}
	
	void setLight(Light l) {light = l;}
	void addSphere(Sphere *s) {spheres.push_back(std::unique_ptr<Sphere>(s));}
	
	bool precomputeSphereInclusion();
	
	Intersection intersect(const Ray &ray) const;
	Vector getColor(const Ray &r, int n, bool fresnel = true) const;
	
private:
	
	std::vector<std::unique_ptr<Sphere> > spheres;
	std::vector<int> sphere_inclusion;
    Light light;
};


#endif
