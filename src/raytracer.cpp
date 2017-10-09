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
	double fov = 60. * PI / 180.;
	int n_bounces = 20;
	double gamma = 2.2; // Gamma correction coefficient

	Scene scene(Light(Vector(-10, 20, 40),1800));
	scene.spheres.push_back(Sphere(Vector(0,0,20),10,Materials::light_glass));
	scene.spheres.push_back(Sphere(Vector(0,3,13),2,Materials::glass));
	scene.spheres.push_back(Sphere(Vector(0,1,17),2,Materials::glass));
	scene.spheres.push_back(Sphere(Vector(0,-1,13),2,Materials::glass));
	scene.spheres.push_back(Sphere(Vector(0,-3,17),2,Materials::glass));
	
	scene.spheres.push_back(Sphere(Vector(0,1000,0),940,Materials::red));
	scene.spheres.push_back(Sphere(Vector(0,0,1000),940,Materials::yellow));
	scene.spheres.push_back(Sphere(Vector(0,-1000,0),990,Materials::blue));
	scene.spheres.push_back(Sphere(Vector(0,0,-1000),940,Materials::green));
	scene.spheres.push_back(Sphere(Vector(1000,0,0),940, Materials::magenta));
	scene.spheres.push_back(Sphere(Vector(-1000,0,0),940, Materials::cyan));

	if (!scene.precomputeSphereInclusion()) {
		ErrorMessage() << "invalid scene! At least two spheres intersect without one being strictly included into the other.";
		return EXIT_FAILURE;
	}
	
	/* Rendering for all pixel */
    #pragma omp parallel for schedule(dynamic,1)
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			Ray ray(camera,Vector(j+0.5-width/2, i+0.5-height/2,-height/(2*tan(fov/2))).normalize()); // Computing ray for pixel (i,j)
			
			Vector color = scene.getColor(ray, n_bounces);
			
			img[((height-i-1)*width+j)] = std::min(255, (int) (255. * pow(color.x,1./gamma)));
			img[((height-i-1)*width+j) + height*width] = std::min(255, (int) (255. * pow(color.y,1./gamma)));
			img[((height-i-1)*width+j) + 2*height*width] = std::min(255, (int) (255. * pow(color.z,1./gamma)));
		}
	}
	
	cimg_library::CImg<unsigned char> cimg(&img[0], width, height, 1, 3);
	cimg.save("fichier.bmp");
	cimg.display();

	
	free(img);
	return 0;
}
