#include "utils.hpp"
#include "scene.hpp"
#include <algorithm>
#include <math.h>
#include <iostream>

bool Scene::precomputeSphereInclusion() {
	// First sort spheres by radius
	std::sort(spheres.begin(), spheres.end(), Sphere::compareByRadius);
	sphere_inclusion = std::vector<int>(spheres.size(),0);
	
	for (int i = 1; i< (long) spheres.size(); i++) {
	    sphere_inclusion[i] = i;
		if (spheres[i]->material.refraction > 0) {
			for (int j = i-1; j >= 0; j--) {
				if (spheres[j]->material.refraction <= 0)
					continue;
				double dist = (spheres[i]->origin - spheres[j]->origin).norm();
				if (dist < spheres[j]->radius - spheres[i]->radius) {
					sphere_inclusion[i] = j;
					break;
				}
				if (dist < spheres[j]->radius + spheres[i]->radius)
					return false;
			}
		}
	}
	return true;
}

Scene::Intersection Scene::intersect(const Ray &ray) const {
	double t = 0;
	int s = 0;
	bool entrance = false;
	for (int i = 0; i< (long) spheres.size(); i++) {
		Sphere::Intersection inter = spheres[i]->intersect(ray);
		// Take minimum of positive t's
		if (inter.first > 0 && (t == 0 || inter.first < t)) {
			t = inter.first;
			s = i;
			entrance = inter.second;
		}
	}
	
	Scene::Intersection inter;
	inter.t = t;
	inter.sphere = s;
	inter.entrance = entrance;
	
	return inter;
}

Vector Scene::getColor(const Ray &ray, int n, bool fresnel) const {
	Vector res(0,0,0);
	double eps = 0.001; // Use to avoid noise
	Scene::Intersection inter = intersect(ray);
	double &t = inter.t;
	const Sphere &sphere = *spheres[inter.sphere];
	
	if (t > 0) { // If it intersects, compute the color of the pixel
		Vector P = ray.origin + t * ray.direction;
		Vector inc = (P - ray.origin).normalize();
		Vector nor = (P - sphere.origin).normalize();
		Vector P1 = P + eps * nor;		
		
		double specularity = sphere.material.specularity;
		double refraction = sphere.material.refraction;
		
		// Compute Fresnel coefficients
		if (fresnel && (sphere_inclusion[inter.sphere] == inter.sphere || sphere.material.refr_index != spheres[sphere_inclusion[inter.sphere]]->material.refr_index) && sphere.material.refraction > 0. && sphere.material.specularity < 0.) {
			double n_inc, n_out;
			Vector nor_refl = nor;
			if (inter.entrance) {
				n_inc = 1.;
				if (sphere_inclusion[inter.sphere] != inter.sphere)
					n_inc = spheres[sphere_inclusion[inter.sphere]]->material.refr_index;
				n_out = sphere.material.refr_index;
			}
			else {
				n_inc = sphere.material.refr_index;
				n_out = 1.;
				if (sphere_inclusion[inter.sphere] != inter.sphere)
					n_out = spheres[sphere_inclusion[inter.sphere]]->material.refr_index;
				nor_refl = - nor_refl;
			}
			double k0 = (n_inc-n_out)*(n_inc-n_out)/((n_inc+n_out)*(n_inc+n_out));
			double l = nor_refl.sp(inc);
			if (l < 0.)
				l = -l;
			refraction = 1.-(k0 + (1.-k0) * pow(1.-l, 5));
			specularity = 1. - refraction;
		}
		
		// If need to bounce and to do refraction, choose one randomly
		if (refraction > 0 && specularity > 0) {
			if (getUniformNumber() < refraction) {
				specularity = 0.;
				refraction = 1.;
			}
			else {
				specularity = 1.;
				refraction = 0.;
			}
		}
			
		if (inter.entrance && specularity > 0 && n>0) { // If specular, bounce if possible
			Ray r;
			r.origin = P1;
			r.direction = (inc - 2 * inc.sp(nor) * nor).normalize();
			res = getColor(r, n-1, fresnel);
			res = specularity * res * sphere.material.spec_color;
		}
		
		if (refraction > 0. && n>0) { // If refraction, compute the refracted ray			
			// First simulate entrance of the sphere
			Ray r;
			double n_inc, n_out;
			Vector nor_refl = nor;
			
			if (inter.entrance) {
				r.origin = P - eps * nor;
				n_inc = 1.;
				if (sphere_inclusion[inter.sphere] != inter.sphere)
					n_inc = spheres[sphere_inclusion[inter.sphere]]->material.refr_index;
				n_out = sphere.material.refr_index;
			} 
			else {
				r.origin = P + eps * nor;
				n_inc = sphere.material.refr_index;
				n_out = 1.;
				if (sphere_inclusion[inter.sphere] != inter.sphere)
					n_out = spheres[sphere_inclusion[inter.sphere]]->material.refr_index;
				nor_refl = - nor_refl;
			}
			
			double norm_coeff = 1 - pow(n_inc/n_out,2) * (1 - pow(inc.sp(nor_refl),2));
			if (norm_coeff >= 0) {  // only reflexion in fact if it is negative
				r.direction = ((n_inc / n_out) * inc - ((n_inc / n_out) * inc.sp(nor_refl) + sqrt(norm_coeff)) * nor_refl).normalize();
				Vector color = getColor(r, n-1, fresnel);
				if (inter.entrance)
					res = res + refraction * color * sphere.material.refr_color;
				else
					res = res + color;
			}
			else {
				r.origin = P1;
				r.direction = (inc - 2 * inc.sp(nor) * nor).normalize();
				res = getColor(r, n-1, fresnel);
				res = refraction * res * sphere.material.refr_color;
			}
		}
		
		
		// Treat the diffuse part
		if (inter.entrance && specularity + refraction < 1) {
		    /* First compute direct lightning */ 
			// If there is an obstacle on the path to the light
			Scene::Intersection obstacle = intersect(Ray(P1,(light.position-P1).normalize()));
			if (obstacle.t <= 0 || obstacle.t > (light.position-P1).norm()) {
				double c = std::max(0., (light.position-P).normalize().sp(nor) * light.intensity / ((light.position-P).snorm()));
				
				res = res + c * (1 - specularity -refraction) * sphere.color(P);
			}
			
			/* Now compute indirect lightning */
			if (n > 0) {
				Ray r;
				r.origin = P1;
				r.direction = generateUniformRandomVector(); // Get random direction
				Vector v = Vector(-nor.y, nor.x, 0).normalize();
				Vector w = nor.vp(v);
				r.direction.convertCoordinateSystem(v, w, nor); // Convert to canonical coordinates
				r.direction = r.direction.normalize();
				res = res + (1./PI) * sphere.material.diffusion_coeff * sphere.color(P) * getColor(r, n-1, fresnel);
			}
		}
	}
	
	return res;
}

/*
Vector Scene::getColor(const Ray &ray, int n) {
	Vector res;
	double eps = 0.001; // Use to avoid noise
	Scene::Intersection inter = intersect(ray);
	double &t = inter.t;
	Sphere &sphere = inter.sphere;
	
	if (t > 0) { // If it intersects, compute the color of the pixel
		Vector P = ray.origin + t * ray.direction;
		Vector inc = (P - ray.origin).normalize();
		Vector nor = (P - sphere.origin).normalize();
		Vector P1 = P + eps * nor;		
		
		if (sphere.material.specularity > 0 && n>0) { // If specular, bounce if possible
			Ray r;
			r.origin = P1;
			r.direction = (inc - 2 * inc.sp(nor) * nor).normalize();
			res = getColor(r, n-1);
			res = sphere.material.specularity * res * sphere.material.spec_color;
		}
		
		if (sphere.material.refraction > 0 && n>0) { // If refraction, compute the refracted ray
			// First simulate entrance of the sphere
			Ray r;
			r.origin = P - eps * nor;
			double n_inc, n_out;
	    
			n_inc = 1;
			n_out = sphere.material.refr_index;

			double norm_coeff = 1 - pow(n_inc/n_out,2) * (1 - pow(inc.sp(nor),2));
			if (norm_coeff >= 0) {  // only reflexion in fact if it is negative
				r.direction = ((n_inc / n_out) * inc - ((n_inc / n_out) * inc.sp(nor) + sqrt(norm_coeff)) * nor).normalize();
				// Then compute exit ray out of the sphere
				Sphere::Intersection inter_out = sphere.intersect(r);
				Vector P_out = r.origin + inter_out.first * r.direction;
				Vector inc_out = (P_out - r.origin).normalize();
				Vector nor_out = (sphere.origin - P_out).normalize();
				
				norm_coeff = 1 - pow(n_out/n_inc,2) * (1 - pow(inc_out.sp(nor_out),2));
				if (norm_coeff >= 0) {
					r.origin = P_out - eps * nor_out;
					r.direction = ((n_out / n_inc) * inc_out - ((n_out / n_inc) * inc_out.sp(nor_out) + sqrt(norm_coeff)) * nor_out).normalize();
					Vector color = getColor(r, n-1);
					res = res + sphere.material.refraction * color * sphere.material.refr_color;
				}
			}
		}
		
		
		// Treat the diffuse part
		if (sphere.material.specularity + sphere.material.refraction < 1) {
			// Check if there is an obstacle on the path to the light
			Scene::Intersection obstacle = intersect(Ray(P1,(light.position-P1).normalize()));
			if (obstacle.t <= 0 || obstacle.t > (light.position-P1).norm()) {
				double c = std::max(0., (light.position-P).normalize().sp(nor) * light.intensity / ((light.position-P).snorm()));
				
				res = res + c * (1 - sphere.material.specularity -sphere.material.refraction) * sphere.material.color;
			}
		}
	}
	
	return res;
}
*/
