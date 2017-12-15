#include "utils.hpp"
#include "scene.hpp"
#include <algorithm>
#include <math.h>
#include <iostream>
#include <sstream>

bool Scene::precomputeSphereInclusion() {
	// First sort spheres by radius
	std::sort(spheres.begin(), spheres.end(), Sphere::compareByRadius);
	sphere_inclusion = std::vector<int>(spheres.size(),0);
	
	for (int i = 1; i< (long) spheres.size(); i++) {
	    sphere_inclusion[i] = i; // Initially set a sphere to be included into itself
		if (spheres[i]->material.refraction > 0) { // We only need to compute sphere inclusion for transparent spheres
			for (int j = i-1; j >= 0; j--) {
				if (spheres[j]->material.refraction <= 0) // Again, only consider transparent spheres
					continue;
				double dist = (spheres[i]->origin - spheres[j]->origin).norm();
				if (dist < spheres[j]->radius - spheres[i]->radius) { // If the sphere is included into the other, store the info
					sphere_inclusion[i] = j;
					break;
				}
				// If the spheres intersect without one being included into the other, there is a problem with the scene
				if (dist < spheres[j]->radius + spheres[i]->radius) {
					return false;
				}
			}
		}
	}
	return true;
}

double Scene::MaxRadiusNewSphere(const Vector &origin) const {
	double r_max = std::numeric_limits<double>::max();
	for (int i = 0; i<(long) spheres.size(); i++) {
		// Check if the point is in the sphere
		double dist = (spheres[i]->origin - origin).norm();
		if (dist < spheres[i]->radius) {
			// If the point is in a non-transparent sphere, returns -1
			if (spheres[i]->material.refraction > 0)
				return -1;
			// If the sphere is transparent, take the radius into account
			r_max = std::min(r_max, spheres[i]->radius - dist);
		}
		else
			r_max = std::min(r_max, dist - spheres[i]->radius);
	}
	return r_max;
}

std::string Scene::toString(const Vector &camera, std::string name) const {
	std::stringstream s;
	if (name != "")
		s << "# " << name << "\n";
	s << "C " << camera.x << " " << camera.y << " " << camera.z << "\n";
	s << "L " << light.position.x << " " << light.position.y << " " << light.position.z << " "
	  << light.intensity << "\n";
	for (int i = 0; i < (long) spheres.size(); i++) {
		s << "S " << spheres[i]->origin.x << " "
			  << spheres[i]->origin.y << " "
			  << spheres[i]->origin.z << " "
			  << spheres[i]->radius << " ";
		bool found = false;
		// Check for the name of the material of the sphere
		for (const auto &M : Materials::by_name) {
			if (M.second == spheres[i]->material) {
				s << M.first << "\n";
				found = true;
				break;
			}
		}
		if (!found)
			s << "object " 
			  << spheres[i]->material.color.x << " "
			  << spheres[i]->material.color.y << " "
			  << spheres[i]->material.color.z << "\n";
	}
	return s.str();
}

Scene::Intersection Scene::intersect(const Ray &ray) const {
	double t = 0;
	int s = 0;
	bool entrance = false;
	// Check for intersection for all spheres of the scene and keep the closest
	for (int i = 0; i< (long) spheres.size(); i++) {
		// Take intersection with sphere i
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

Vector Scene::getColor(const Ray &ray, int n, bool fresnel, bool diffuse, bool deterministic) const {
	Vector res(0,0,0);
	double eps = 0.001; // Use to avoid noise
	Scene::Intersection inter = intersect(ray); // Retrieve closest intersection between spheres and input ray
	double &t = inter.t;
	const Sphere &sphere = *spheres[inter.sphere];
	
	if (t > 0) { // If it intersects something, compute the color of the pixel (otherwise it will return black)
		Vector P = ray.origin + t * ray.direction; // P is the intersection point between the sphere and input ray
		Vector inc = (P - ray.origin).normalize(); // Incoming vector
		Vector nor = (P - sphere.origin).normalize(); // Normal vector
		// Slightly shifted points to avoid noise effects
		Vector P1 = P + eps * nor;
		Vector P2 = P - eps * nor;
		
		double specularity = sphere.material.specularity;
		double refraction = sphere.material.refraction;
		
		/*** Computing Fresnel coefficients ***
		  If the material of the sphere is (partially) transparent and not specular, if the 
		  no-fresnel option is disable and if the refractive index of the sphere is different from 
		  the refractive index just outside of the sphere, we compute fresnel coefficients according
		  to Schlick's formulas.
		*/
		if (fresnel && (sphere_inclusion[inter.sphere] == inter.sphere || sphere.material.refr_index != spheres[sphere_inclusion[inter.sphere]]->material.refr_index) && sphere.material.refraction > 0 && sphere.material.specularity <= 0.) {
			double n_inc, n_out;
			Vector nor_refl = nor;
			// First compute refractive index of the incoming and outgoing materials
			if (inter.entrance) {
				n_inc = 1.;
				// If the ray enters a sphere wich is included into another sphere, the in refractive index is
				// the index of the sphere it is included into
				if (sphere_inclusion[inter.sphere] != inter.sphere)
					n_inc = spheres[sphere_inclusion[inter.sphere]]->material.refr_index;
				n_out = sphere.material.refr_index;
			}
			else {
				n_inc = sphere.material.refr_index;
				n_out = 1.;
				// If the ray leaves a sphere wich is included into another sphere, the out refractive index is
				// the index of the sphere it is included into
				
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
		
		// If need to bounce and to do refraction, choose only one randomly (according to the coefficients)
		if (!deterministic && refraction > 0 && specularity > 0) {
			if (getUniformNumber() < refraction) {
				specularity = 0.;
				refraction = 1.;
			}
			else {
				specularity = 1.;
				refraction = 0.;
			}
		}
		
		// If specular, bounce if possible
		if (inter.entrance && specularity > 0 && n>0) {
			Ray r;
			r.origin = P1;
			r.direction = (inc - 2 * inc.sp(nor) * nor).normalize();
			res = getColor(r, n-1, fresnel, diffuse); // Compute recursively the color
			res = specularity * res * sphere.material.spec_color; // Multiply by the specularity color
		}
		
		// If refraction, compute the refracted ray
		if (refraction > 0. && n>0) {			
			// First simulate entrance of the sphere
			Ray r;
			double n_inc, n_out;
			Vector nor_refl = nor;
			
			// Compute origin points of the new ray, in/out refractive indices and normal vectors depending
			// on whether we enter or leave the sphere
			if (inter.entrance) {
				r.origin = P2;
				n_inc = 1.;
				if (sphere_inclusion[inter.sphere] != inter.sphere)
					n_inc = spheres[sphere_inclusion[inter.sphere]]->material.refr_index;
				n_out = sphere.material.refr_index;
			} 
			else {
				r.origin = P1;
				n_inc = sphere.material.refr_index;
				n_out = 1.;
				if (sphere_inclusion[inter.sphere] != inter.sphere)
					n_out = spheres[sphere_inclusion[inter.sphere]]->material.refr_index;
				nor_refl = - nor_refl;
			}
			
			double norm_coeff = 1 - pow(n_inc/n_out,2) * (1 - pow(inc.sp(nor_refl),2));
			if (norm_coeff >= 0) {  // only reflexion in fact if the coeff is negative
				r.direction = ((n_inc / n_out) * inc - ((n_inc / n_out) * inc.sp(nor_refl) + sqrt(norm_coeff)) * nor_refl).normalize(); // Direction of refracted ray
				Vector color = getColor(r, n-1, fresnel, diffuse); // Compute the color recursively
				if (inter.entrance) // Apply the multiplicative coeffs only once (at the entrance of the sphere)
					res = res + refraction * color * sphere.material.refr_color;
				else
					res = res + color;
			}
			else { // In that case we have the reflexion
				if (inter.entrance)
					r.origin = P1;
				else
					r.origin = P2;
				r.direction = (inc - 2 * inc.sp(nor) * nor).normalize();
				res = getColor(r, n-1, fresnel, diffuse);
				res = refraction * res * sphere.material.refr_color;
			}
		}
		
		
		// Treat the diffuse part
		if (inter.entrance && specularity + refraction < 1) {
			Scene::Intersection obstacle = intersect(Ray(P1,(light.position-P1).normalize()));
			// If there is no obstacle on the path to the light, compute direct lightning
			if (obstacle.t <= 0 || obstacle.t > (light.position-P1).norm()) {
				double c = std::max(0., (light.position-P).normalize().sp(nor) * light.intensity / ((light.position-P).snorm())); // Compute intensity of light received on the point
				
				res = res + c * (1 - specularity -refraction) * sphere.color(P);
			}
			
			// Now compute indirect lightning
			if (diffuse && n > 0) {
				Ray r;
				r.origin = P1;
				r.direction = generateUniformRandomVector(); // Get random direction
				// Compute local coordinate system in which the direction of the ray is taken randomly
				Vector v = Vector(-nor.y, nor.x, 0).normalize();
				Vector w = nor.vp(v);
				r.direction.convertCoordinateSystem(v, w, nor); // Convert to canonical coordinates
				r.direction = r.direction.normalize();
				res = res + (1./PI) * sphere.material.diffusion_coeff * sphere.color(P) * getColor(r, n-1, fresnel, diffuse); // Compute recursively the color and take diffusion into account
			}
		}
	}
	
	return res;
}
