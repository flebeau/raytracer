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
	int height = 1000;
	int width = 1000;
	
	int *img = (int*)malloc(3 * height * width * sizeof(int));
	
	/* Constants of the scene */
	Vector camera = Vector(0,0,55);
	double fov = 60. * PI / 180.;
	int n_bounces = 5;
	
	Scene scene(Light(Vector(-10, 20, 40),1100));
	scene.spheres.push_back(Sphere(Vector(0,0,0),10,Materials::mirror));
	scene.spheres.push_back(Sphere(Vector(0,1000,0),940,Materials::red));
	scene.spheres.push_back(Sphere(Vector(0,0,1000),940,Materials::yellow));
	scene.spheres.push_back(Sphere(Vector(0,-1000,0),990,Materials::blue));
	scene.spheres.push_back(Sphere(Vector(0,0,-1000),940,Materials::green));
	scene.spheres.push_back(Sphere(Vector(1000,0,0),940));
	scene.spheres.push_back(Sphere(Vector(-1000,0,0),940));
   	
	/* Rendering for all pixel */
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			Ray ray(camera,Vector(j+0.5-width/2, i+0.5-height/2,-height/(2*tan(fov/2))).normalize()); // Computing ray for pixel (i,j)
			
			Vector color = scene.getColor(ray, n_bounces);
			
			img[((height-i-1)*width+j)] = std::min(255, (int) (255. * color.x));
			img[((height-i-1)*width+j) + height*width] = std::min(255, (int) (255. * color.y));
			img[((height-i-1)*width+j) + 2*height*width] = std::min(255, (int) (255. * color.z));
		}
	}
	
	cimg_library::CImg<unsigned char> cimg(&img[0], width, height, 1, 3);
	cimg.save("fichier.bmp");
	cimg.display();

	
	free(img);
	return 0;
}
