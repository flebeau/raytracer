#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <vector>

class Material {
public:
	Material() : color(3) {
		color[0] = 1;
		color[1] = 1;
		color[2] = 1;
	}
	Material(double r, double g, double b) : color(3) {
		color[0] = r;
		color[1] = g;
		color[2] = b;
	}
	
	std::vector<double> color;	
};

/* Materials */

namespace Materials {
	const Material blue = Material(0,0,1);
	const Material red = Material(1,0,0);
	const Material green = Material(0,1,0);
	const Material magenta = Material(1,1,0);
	const Material white = Material(1,1,1);
}

#endif
