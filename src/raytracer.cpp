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
	
	// Parameters for Raytracer
	int height;
	int width;
	int n_bounces = 6;
	int n_retry = 100;
	double fov = 60.;
	std::string scene_file;
	std::string output_file = "";
	bool fresnel = true;
	bool random_scene = false;
	
    // Parameters for random scene generation
	double size_x = 80;
	double size_y = 80;
	double size_z = 80;
	double radius_min = 5;
	double radius_max = 15;
	double mirror_prob = 0.3;
	double transparent_prob = 0.6;
	double object_prob = 0.3;
	unsigned n_spheres = 15;
	
	try {
		po::options_description opt_raytracer("Options for the Raytracer program");
		opt_raytracer.add_options()
			("height,H", po::value<int>(&height)->required(), "Height of generated image (required)")
			("width,W", po::value<int>(&width)->required(), "Width of generated image (required)")
			("bounces,b", po::value<int>(&n_bounces), "Specify number of bounces (default: 6)")
			("rays,r", po::value<int>(&n_retry), "Specify number of rays per pixel (default: 100)")
			("fov,F", po::value<double>(&fov), "Specify fov (in degree) of the camera (default: 60)")
			("scene-file,f", po::value<std::string>(&scene_file)->required(), "Input file containing the scene description (required)")
			("output-file,o", po::value<std::string>(&output_file), "Output image file (if none, output in window)")
			("no-fresnel", "Disable use of Fresnel coefficients")
			;
		
		po::options_description opt_random_scene("Options for random scene generation");
		opt_random_scene.add_options()
			("output_file,o", po::value<std::string>(&output_file), "Output scene file")
			;
		
		po::options_description opt_descr("General options");
		opt_descr.add_options()("help,h", "Display a list of available options")("random-scene,R", "Enable the generation of a random scene file");
		
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(opt_descr).allow_unregistered().run(), vm);
		if (vm.count("random-scene")) {
			random_scene = true;
			opt_descr.add(opt_random_scene);
		}
		else
			opt_descr.add(opt_raytracer);
		
		if (vm.count("help")) {
			std::cerr << opt_descr << "\n";
			return EXIT_SUCCESS;
		}
		
		vm.clear();
		po::store(po::command_line_parser(argc, argv).options(opt_descr).run(), vm);
		
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
	
	Vector camera(0,0,0);
	Vector light(0,0,0);
	Scene scene;
	
	/* Random scene generation */
	if (random_scene) {
		// Normalize given probabilities so that the sum is 1
		double s = mirror_prob + transparent_prob + object_prob; 
		mirror_prob /= s;
		transparent_prob /= s;
		object_prob /= s;
		
		camera = Vector(0,0,size_z/2);
		scene.setLight(Light(Vector(0, size_y/4, size_z/2), 1000));
		
		scene.addSphere(new Sphere(Vector(0,0,-1000-size_z/2), 1000, Materials::green));
		scene.addSphere(new Sphere(Vector(0,1000+size_y/2, 0), 1000, Materials::red));
		scene.addSphere(new Sphere(Vector(0,-1000-size_y/2,0), 1000, Materials::blue));
		scene.addSphere(new Sphere(Vector(1000+size_x/2,0,0), 1000, Materials::magenta));
		scene.addSphere(new Sphere(Vector(-1000-size_x/2,0,0), 1000, Materials::cyan));
		
		unsigned spheres_created = 0;
		while (spheres_created < n_spheres) {
			// First choose the origin randomly
			Vector origin(getUniformNumber() * size_x - size_x/2
						  ,getUniformNumber() * size_y - size_y/2
						  ,getUniformNumber() * size_z - size_z/2);
			// Determine max radius such that the sphere is in the domain
			double r_max = std::min(radius_max, std::min(size_x/2 - abs(origin.x)
											  , std::min(size_y/2 - abs(origin.y)
														 ,size_z/2 - abs(origin.z))));
			double m = scene.MaxRadiusNewSphere(origin);
			if (m == -1) // If the point is included in a non-transparent sphere, skip
				continue;
			r_max = std::min(r_max, m);
			// If the max radius is less than the min radius, the origin is invalid
			if (r_max < radius_min)
				continue;
			// Else we can create the sphere, by choosing the material at random
			double radius = getUniformNumber() * (r_max - radius_min) + radius_min;
			double t = getUniformNumber();
			if (t < mirror_prob)
				scene.addSphere(new Sphere(origin, radius, Materials::mirror));
			else if (t < mirror_prob + transparent_prob)
				scene.addSphere(new Sphere(origin, radius, Materials::glass));
			else
				scene.addSphere(new Sphere(origin, radius, Material(getUniformNumber(), getUniformNumber(), getUniformNumber())));
			++spheres_created;
		}
		
		if (output_file == "")
			std::cout << scene.toString(camera, "Random scene");
		
		return EXIT_SUCCESS;
	}
	
	/* Loading the scene */
	std::ifstream scene_spec;
	scene_spec.open(scene_file);
	double x, y, z, intensity, radius, r, g, b;
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
			else if (material == "object") {
				s >> r >> g >> b;
				scene.addSphere(new Sphere(Vector(x,y,z),radius,Material(r,g,b)));
			}
			else {
				scene.addSphere(new Sphere(Vector(x,y,z),radius,Materials::by_name.at(material)));
			}
			continue;
		}
	}
	
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
