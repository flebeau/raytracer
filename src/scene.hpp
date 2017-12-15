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

/** Main class of the raytracer
	Contains the scene (light, spheres) and methods computing the color of a given ray
*/
class Scene {
public:
	// Struct containing all the info needed to compute intersection between a ray and a scene
	struct Intersection {
		double t;
		int sphere;
		bool entrance;
	};
	
	Scene() : light(Vector(0,0,0),0) {}
	Scene(Light l) : light(l) {}
	
	void setLight(Light l) {light = l;}
	void addSphere(Sphere *s) {spheres.push_back(std::unique_ptr<Sphere>(s));}
	
	// Fill the vector sphere_inclusion such that spere_inclusion[i] = j if and only if
	// the j-th sphere is the smallest sphere containing the i-th sphere.
	// We use this precomputation step in order to obtain the refractive index outside of all spheres
	// If there are two spheres which intersect and such that none of the two is included into the other,
	// this method returns false (the scene is invalid!)
	bool precomputeSphereInclusion();
	
    // Returns the maximum radius of a sphere with the given origin such that
	// the sphere would be included in existing spheres, and returns -1 if the origin
	// is in a non-transparent sphere
	double MaxRadiusNewSphere(const Vector &origin) const;
	
	// Export scene to string according to the specification format
	std::string toString(const Vector &camera, std::string name = "") const; 
	
	// Compute the closest intersection between the input ray and all the spheres of the scene
	Intersection intersect(const Ray &ray) const;
	
	// Main function of the raytracer: returns the color of the input ray simulated in the scene with
	// recursion depth n. The use of Fresnel coefficients can be toggled off.
	Vector getColor(const Ray &r, int n, bool fresnel = true, bool diffuse = true, bool deterministic = false) const;
	
private:
	std::vector<std::unique_ptr<Sphere> > spheres;
	std::vector<int> sphere_inclusion;
    Light light;
};


#endif
