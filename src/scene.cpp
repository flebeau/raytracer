#include "scene.hpp"
#include <algorithm>

Scene::Intersection Scene::intersect(const Ray &ray) const {
	double t = 0;
	int s = 0;
	for (int i = 0; i<spheres.size(); i++) {
		double temp = spheres[i].intersect(ray);
		if (temp > 0 && (t == 0 || temp < t)) {
			t = temp;
			s = i;
		}
	}
	
	return Scene::Intersection(t, spheres[s]);
}
