#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "vector.hpp"

class Material {
public:
	Material() : color(1,1,1), specularity(0) {}
	Material(double r, double g, double b, double s = 0, double sr = 1, double sg = 1, double sb = 1) : color(r,g,b), specularity(s), spec_color(sr,sg,sb) {}
	
	Vector color;
	Vector spec_color;
	double specularity;
};

/* Materials */

namespace Materials {
	const Material blue = Material(0,0,1);
	const Material red = Material(1,0,0);
	const Material green = Material(0,1,0);
	const Material magenta = Material(1,0,1);
	const Material yellow = Material(1,1,0);
	const Material mirror = Material(0,0,0,1.);
	const Material yellow_mirror = Material(1,1,0,0.5,1,1,0);
}

#endif
