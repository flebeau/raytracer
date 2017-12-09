#include <iostream>
#include <math.h>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "vector.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "utils.hpp"
#include "scene.hpp"
#include <boost/program_options.hpp>
#include <boost/progress.hpp>

#include "CImg.h"

int main(int argc, char *argv[]) {
	namespace po = boost::program_options;
	
	int height;
	int width;
	int n_bounces = 6;
	int n_retry = 100;
	double fov = 60.;
	std::string scene_file;
	std::string output_file = "";
	bool fresnel = true;
	
	try {
		po::options_description opt_descr("Options description");
		opt_descr.add_options()
			("help,h", "Display this help message")
			("height,H", po::value<int>(&height)->required(), "Height of generated image")
			("width,W", po::value<int>(&width)->required(), "Width of generated image")
			("bounces,b", po::value<int>(&n_bounces), "Specify number of bounces")
			("rays,r", po::value<int>(&n_retry), "Specify number of rays per pixel")
			("fov,F", po::value<double>(&fov), "Specify fov (in degree) of the camera")
			("scene-file,f", po::value<std::string>(&scene_file)->required(), "Input file containing the scene description")
			("output-file,o", po::value<std::string>(&output_file), "Output image file")
			("no-fresnel", "Disable use of Fresnel coefficients")
			;
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(opt_descr).run(), vm);
		
		if (vm.count("help")) {
			std::cerr << opt_descr << "\n";
			return EXIT_SUCCESS;
		}
		if (vm.count("no-fresnel"))
			fresnel = false;
		
		po::notify(vm);		
	}
	catch(std::exception &e) {
		std::cerr << "Error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
	fov *= PI / 180.;
	double gamma = 2.2; // Gamma correction coefficient
	
	/* Image initialization */
	int *img = (int*)malloc(3 * height * width * sizeof(int));
	
	/* Loading the scene */
	Vector camera(0,0,0);
	Vector light(0,0,0);
	Scene scene;
	
	std::ifstream scene_spec;
	scene_spec.open(scene_file);
	double x, y, z, intensity, radius;
	std::string material;
	std::string line;
	unsigned n_line = 0;
	while (getline(scene_spec, line)) {
		++n_line;
		if (line[0] == '#')
			continue;
		if (line[0] == 'L') {
			std::stringstream s(line.substr(1));
			if (!(s >> x >> y >> z >> intensity)) {
				ErrorMessage() << "parsing error at line " << n_line << " of file " << scene_file;
				exit(EXIT_FAILURE);
			}
			light = Vector(x, y, z);
			scene.setLight(Light(light, intensity));
			continue;
		}
		if (line[0] == 'C') {
			std::stringstream s(line.substr(1));
			if (!(s >> x >> y >> z)) {
				ErrorMessage() << "parsing error at line " << n_line << " of file " << scene_file;
				exit(EXIT_FAILURE);
			}
			camera = Vector(x, y, z);
			continue;
		}
		if (line[0] == 'S') {
			std::stringstream s(line.substr(1));
			material = "";
			s >> x >> y >> z >> radius >> material;
			if (material == "multicolor") {
				scene.addSphere(new MultiColorSphere(Vector(x,y,z),radius));
			}
			else {
				scene.addSphere(new Sphere(Vector(x,y,z),radius,Materials::by_name.at(material)));
			}
			continue;
		}
	}
	
	//Vector camera = Vector(0,0,55);
	
	//Scene scene(Light(Vector(-10, 20, 40),3000));
	//scene.addSphere(new Sphere(Vector(0,0,20),10, Materials::glass));
	//scene.addSphere(new MultiColorSphere(Vector(0,0,20),10));
    //scene.spheres.push_back(Sphere(Vector(0,0,20),6, Materials::light_glass));
	// scene.spheres.push_back(Sphere(Vector(0,3,13),2,Materials::glass));
	// scene.addSphere(Sphere(Vector(0,1,17),2,Materials::glass));
	// scene.addSphere(Sphere(Vector(0,-1,13),2,Materials::glass));
	// scene.addSphere(Sphere(Vector(0,-3,17),2,Materials::glass));
	
	//scene.addSphere(new Sphere(Vector(0,1000,0),940,Materials::red));
	//scene.addSphere(new Sphere(Vector(0,0,1000),940));
	//scene.addSphere(new Sphere(Vector(0,-1000,0),990,Materials::blue));
	//scene.addSphere(new Sphere(Vector(0,0,-1000),940,Materials::green));
	//scene.addSphere(new Sphere(Vector(1000,0,0),940, Materials::magenta));
	//scene.addSphere(new Sphere(Vector(-1000,0,0),940, Materials::cyan));

	if (!scene.precomputeSphereInclusion()) {
		ErrorMessage() << "invalid scene! At least two spheres intersect without one being strictly included into the other.";
		return EXIT_FAILURE;
	}
	
	/* Rendering for all pixel */
	boost::progress_display progress((unsigned long) height+1, std::cerr);
	++progress;
    #pragma omp parallel for schedule(dynamic,1)
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			Ray ray(camera,Vector(j+0.5-width/2, i+0.5-height/2,-height/(2*tan(fov/2))).normalize()); // Computing ray for pixel (i,j)
			
			Vector color = Vector(0, 0, 0);
			for (int n = 0; n<n_retry; n++) {
				color = color + scene.getColor(ray, n_bounces, fresnel);
			}
			color = (1. / ((double)n_retry)) * color;
			
			img[((height-i-1)*width+j)] = std::min(255, (int) (255. * pow(color.x,1./gamma)));
			img[((height-i-1)*width+j) + height*width] = std::min(255, (int) (255. * pow(color.y,1./gamma)));
			img[((height-i-1)*width+j) + 2*height*width] = std::min(255, (int) (255. * pow(color.z,1./gamma)));
		}
		++progress;
	}
	
	cimg_library::CImg<unsigned char> cimg(&img[0], width, height, 1, 3);
	if (output_file != "")
		cimg.save(output_file.c_str());
	else
		cimg.display();
	
	
	free(img);
	return 0;
}
