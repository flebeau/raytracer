#include <iostream>
#include <math.h>

#include "vector.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "utils.hpp"

#include "CImg.h"

int main(int argc, char *argv[]) {
	/* Image initialization */
	int height = 255;
	int width = 255;
	
	int *img = (int*)malloc(3 * height * width * sizeof(int));
	
	/* Constants of the scene */
	Vector camera = Vector(0,0,55);
	double fov = 60. * PI / 180.;
	Sphere subject = Sphere(Vector(0,0,0), 10);
	
	
	/* Rendering for all pixel */
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			Ray ray(camera,Vector(j+0.5-width/2, i+0.5-height/2,-height/(2*tan(fov/2))).normalize()); // Computing ray for pixel (i,j)
			
			// If it intersects, set its color to white
			if (subject.intersect(ray) > 0) {
				img[((height-i-1)*width+j)] = 255;
				img[((height-i-1)*width+j) + height*width] = 255;
				img[((height-i-1)*width+j) + 2*height*width] = 255;
			}
			else {
				img[((height-i-1)*width+j)] = 0;
				img[((height-i-1)*width+j) + height*width] = 0;
				img[((height-i-1)*width+j) + 2*height*width] = 0;
			}
			
			
		}
	}
	
	cimg_library::CImg<unsigned char> cimg(&img[0], width, height, 1, 3);
	cimg.save("fichier.bmp");

	
	free(img);
	return 0;
}
