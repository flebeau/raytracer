#include <iostream>
#include <math.h>
#include <algorithm>

#include "vector.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "utils.hpp"
#include "scene.hpp"

#include "CImg.h"

int main(int argc, char *argv[]) {
	/* Image initialization */
	int height = 2000;
	int width = 2000;
	
	int *img = (int*)malloc(3 * height * width * sizeof(int));
	
	/* Constants of the scene */
	Vector camera = Vector(0,0,55);
	double fov = 70. * PI / 180.;
	double eps = 0.001;
	
	Scene scene;
	scene.spheres.push_back(Sphere(Vector(0,0,0),10));
	scene.spheres.push_back(Sphere(Vector(0,1000,0),940,Materials::red));
	scene.spheres.push_back(Sphere(Vector(0,0,1000),940,Materials::magenta));
	scene.spheres.push_back(Sphere(Vector(0,-1000,0),990,Materials::blue));
	scene.spheres.push_back(Sphere(Vector(0,0,-1000),940,Materials::green));
	scene.spheres.push_back(Sphere(Vector(1000,0,0),940));
	scene.spheres.push_back(Sphere(Vector(-1000,0,0),940));
	
	
	Vector light = Vector(-10, 20, 40);
	double intensity = 900;
	
	/* Rendering for all pixel */
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			Ray ray(camera,Vector(j+0.5-width/2, i+0.5-height/2,-height/(2*tan(fov/2))).normalize()); // Computing ray for pixel (i,j)
			
			// If it intersects, compute the color of the pixel
			Scene::Intersection inter = scene.intersect(ray);
			double &t = inter.first;
			Sphere &subject = inter.second;
			if (t > 0) {
				Vector P = camera + t * ray.direction;
				Vector P1 = P + eps * (P-subject.origin).normalize();
				// Check if there is an obstacle on the path to the light
				Scene::Intersection obstacle = scene.intersect(Ray(P1,(light-P1).normalize()));
				if (obstacle.first > 0 && obstacle.first <= (light-P1).norm()) { // if there is set it to black
					img[((height-i-1)*width+j)] = 0;
					img[((height-i-1)*width+j) + height*width] = 0;
					img[((height-i-1)*width+j) + 2*height*width] = 0;
				}
				else {
				
					double c = std::max(0., (light-P).normalize().sp((P-subject.origin).normalize())) * intensity / ((light-P).snorm());
					
					c *= 255.;
					
					img[((height-i-1)*width+j)] = (int) (c * subject.material.color[0]);
					img[((height-i-1)*width+j) + height*width] = (int) (c * subject.material.color[1]);
					img[((height-i-1)*width+j) + 2*height*width] = (int) (c * subject.material.color[2]);
				}
			}
			else { // Otherwise set it to black
				img[((height-i-1)*width+j)] = 0;
				img[((height-i-1)*width+j) + height*width] = 0;
				img[((height-i-1)*width+j) + 2*height*width] = 0;
			}
			
			
		}
	}
	
	cimg_library::CImg<unsigned char> cimg(&img[0], width, height, 1, 3);
	cimg.save("fichier.bmp");
	cimg.display();

	
	free(img);
	return 0;
}
