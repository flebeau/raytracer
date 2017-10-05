#include "scene.hpp"
#include <algorithm>

Scene::Intersection Scene::intersect(const Ray &ray) const {
	double t = 0;
	int s = 0;
	for (int i = 0; i<spheres.size(); i++) {
		double temp = spheres[i].intersect(ray);
		// Take minimum of positive t's
		if (temp > 0 && (t == 0 || temp < t)) {
			t = temp;
			s = i;
		}
	}
	
	return Scene::Intersection(t, spheres[s]);
}

Vector Scene::getColor(const Ray &ray, int n) {
	Vector res;
	double eps = 0.001; // Use to avoid noise
	Scene::Intersection inter = intersect(ray);
	double &t = inter.first;
	Sphere &sphere = inter.second;
	
	if (t > 0) { // If it intersects, compute the color of the pixel
		Vector P = ray.origin + t * ray.direction;
		Vector P1 = P + eps * (P-sphere.origin).normalize();
		if (sphere.material.specularity > 0 && n>0) { // If specular, bounce if possible
			Ray r;
			r.origin = P1;
			Vector inc = P - ray.origin;
			Vector nor = P - sphere.origin;
			r.direction = (inc - 2 * inc.sp(nor) * nor).normalize();
			res = getColor(r, n-1);
			res = sphere.material.specularity * res * sphere.material.spec_color;
		}
		// Treat the diffuse part
		if (sphere.material.specularity < 1) {
			// Check if there is an obstacle on the path to the light
			Scene::Intersection obstacle = intersect(Ray(P1,(light.position-P1).normalize()));
			if (obstacle.first <= 0 || obstacle.first > (light.position-P1).norm()) {
				double c = std::max(0., (light.position-P).normalize().sp((P-sphere.origin).normalize())) * light.intensity / ((light.position-P).snorm());
				
				res = res + c * (1-sphere.material.specularity) * sphere.material.color;
			}
		}
	}
	
	return res;
}
