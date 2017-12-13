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
	bool antialiasing = false;
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
	double intensity = 1000;

	// Handle options
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
			("antialiasing", "Enable antialiasing")
			;
		
		po::options_description opt_random_scene("Options for random scene generation");
		opt_random_scene.add_options()
			("size-x,x", po::value<double>(&size_x), "Size of the domain in x coordinate (default: 80)")
			("size-y,y", po::value<double>(&size_y), "Size of the domain in y coordinate (default: 80)")
			("size-z,z", po::value<double>(&size_z), "Size of the domain in z coordinate (default: 80)")
			("radius-min", po::value<double>(&radius_min), "Minimum radius of generated spheres (default: 5)")
			("radius-max", po::value<double>(&radius_max), "Maximum radius of generated spheres (default: 15)")
			("mirror,m", po::value<double>(&mirror_prob), "Proportion of mirror spheres (default: 0.3)")
			("transparent,t", po::value<double>(&transparent_prob), "Proportion of transparent spheres (default: 0.6)")
			("object,b", po::value<double>(&object_prob), "Proportion of object spheres (default: 0.3)")
			("n-sphere,n", po::value<unsigned>(&n_spheres), "Number of generated spheres (default: 15)")
			("intensity,i", po::value<double>(&intensity), "Intensity of the light (default: 1000)")
			;
		
		po::options_description opt_descr("General options");
		opt_descr.add_options()("help,h", "Display a list of available options")("random-scene,R", "Enable the generation of a random scene file");
		
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(opt_descr).allow_unregistered().run(), vm);
		
		// If random scene generation is selected, only consider options for random generation
		if (vm.count("random-scene")) {
			random_scene = true;
			opt_descr.add(opt_random_scene);
		}
		else // Otherwise consider options for the raytracer
			opt_descr.add(opt_raytracer);
		
		// Print help message if option --help
		if (vm.count("help")) {
			std::cerr << opt_descr << "\n";
			return EXIT_SUCCESS;
		}
		
		vm.clear();
		po::store(po::command_line_parser(argc, argv).options(opt_descr).run(), vm);
		
		if (vm.count("no-fresnel"))
			fresnel = false;
		if (vm.count("antialiasing"))
			antialiasing = true;
		
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
		// Normalize given proportions so that the sum is 1
		double s = mirror_prob + transparent_prob + object_prob; 
		mirror_prob /= s;
		transparent_prob /= s;
		object_prob /= s;
		
		// Set the camera and light
		camera = Vector(0,0,size_z/2);
		scene.setLight(Light(Vector(0, size_y/4, size_z/2), intensity));

		// Add walls
		scene.addSphere(new Sphere(Vector(0,0,1000+3*size_z/4), 1000, Materials::neutral));
		scene.addSphere(new Sphere(Vector(0,0,-1000-size_z/2), 1000, Materials::green));
		scene.addSphere(new Sphere(Vector(0,1000+size_y/2, 0), 1000, Materials::red));
		scene.addSphere(new Sphere(Vector(0,-1000-size_y/2,0), 1000, Materials::blue));
		scene.addSphere(new Sphere(Vector(1000+size_x/2,0,0), 1000, Materials::magenta));
		scene.addSphere(new Sphere(Vector(-1000-size_x/2,0,0), 1000, Materials::cyan));
		
		// Now generate spheres
		unsigned spheres_created = 0;
		unsigned failures = 0;
		while (spheres_created < n_spheres) {
			if (failures > 100) {
				ErrorMessage() << "Could not generate random scene: too many failures!";
				exit(EXIT_FAILURE);
			}
			// First choose the origin randomly
			Vector origin(getUniformNumber() * size_x - size_x/2
						  ,getUniformNumber() * size_y - size_y/2
						  ,getUniformNumber() * size_z - size_z/2);
			// Determine max radius such that the sphere is in the domain
			double r_max = std::min(radius_max, std::min(size_x/2 - abs(origin.x)
											  , std::min(size_y/2 - abs(origin.y)
														 ,size_z/2 - abs(origin.z))));
			double m = scene.MaxRadiusNewSphere(origin);
			if (m == -1) { // If the point is included in a non-transparent sphere, skip
				++failures;
				continue;
			}
			r_max = std::min(r_max, m);
			// If the max radius is less than the min radius, the origin is invalid
			if (r_max < radius_min) {
				++failures;
				continue;
			}
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
			failures = 0;
		}
		
		std::cout << scene.toString(camera, "Random scene");
		
		return EXIT_SUCCESS;
	}
	
	/* Loading the scene */
	std::ifstream scene_spec;
	scene_spec.open(scene_file);
	double x, y, z, radius, r, g, b;
	std::string material;
	std::string line;
	unsigned n_line = 0;
	// We read the file line by line
	while (getline(scene_spec, line)) {
		++n_line;
		if (line[0] == '#') // Skip commentary lines
			continue;
		if (line[0] == 'L') { // Handle light specification
			std::stringstream s(line.substr(1));
			if (!(s >> x >> y >> z >> intensity)) {
				ErrorMessage() << "parsing error at line " << n_line << " of file " << scene_file;
				exit(EXIT_FAILURE);
			}
			light = Vector(x, y, z);
			scene.setLight(Light(light, intensity));
			continue;
		}
		if (line[0] == 'C') { // Handle camera specification
			std::stringstream s(line.substr(1));
			if (!(s >> x >> y >> z)) {
				ErrorMessage() << "parsing error at line " << n_line << " of file " << scene_file;
				exit(EXIT_FAILURE);
			}
			camera = Vector(x, y, z);
			continue;
		}
		if (line[0] == 'S') { // Handle sphere specification
			std::stringstream s(line.substr(1));
			material = "";
			s >> x >> y >> z >> radius >> material;
			if (material == "multicolor") {
				scene.addSphere(new MultiColorSphere(Vector(x,y,z),radius));
			}
			else if (material == "object") { // Can specify the color of an object
				s >> r >> g >> b;
				scene.addSphere(new Sphere(Vector(x,y,z),radius,Material(r,g,b)));
			}
			else {
				scene.addSphere(new Sphere(Vector(x,y,z),radius,Materials::by_name.at(material)));
			}
			continue;
		}
	}
	
	// First precompute sphere inclusions and check that spheres are either contained in another or disjoint from another one
	if (!scene.precomputeSphereInclusion()) {
		ErrorMessage() << "invalid scene! At least two spheres intersect without one being strictly included into the other.";
		return EXIT_FAILURE;
	}
	
	/* Rendering for all pixel */
	std::cerr << "### Treating scene file " << scene_file << " ###";
	boost::progress_display progress((unsigned long) height+1, std::cerr); // Progress bar
	++progress;
    #pragma omp parallel for schedule(dynamic,1)
	for (int i = 0; i<height; i++) {
		for (int j = 0; j<width; j++) {
			Ray ray;
			// Computing ray for pixel (i,j) with or without antialiasing
			if (!antialiasing)
				ray = Ray(camera,Vector(j+0.5-width/2, i+0.5-height/2,-height/(2*tan(fov/2))).normalize()); 
			else {
				double x = getUniformNumber(); 
				double y = getUniformNumber();
				double R = sqrt(-2*log(x));
				double u = R * cos(2 * PI * y) * 0.5;
				double v = R * sin(2 * PI * y) * 0.5;
				ray = Ray(camera, Vector(j+u-width/2-0.5, i+v-height/2-0.5,-height/(2*tan(fov/2))).normalize());
			}
			// Simulate with n_retry rays and do the mean
			Vector color = Vector(0, 0, 0);
			for (int n = 0; n<n_retry; n++) {
				color = color + scene.getColor(ray, n_bounces, fresnel);
			}
			color = (1. / ((double)n_retry)) * color;
			
			// Write the result in the image, applying gamma correction and ensuring that the output color is between 0 and 255
			img[((height-i-1)*width+j)] = std::min(255, (int) (255. * pow(color.x,1./gamma)));
			img[((height-i-1)*width+j) + height*width] = std::min(255, (int) (255. * pow(color.y,1./gamma)));
			img[((height-i-1)*width+j) + 2*height*width] = std::min(255, (int) (255. * pow(color.z,1./gamma)));
		}
		++progress;
	}
	
	// Output in a file or display generated image
	cimg_library::CImg<unsigned char> cimg(&img[0], width, height, 1, 3);
	if (output_file != "")
		cimg.save(output_file.c_str());
	else
		cimg.display();
	
	// Free memory and return
	free(img);
	return 0;
}
