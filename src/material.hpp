#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "vector.hpp"

class Material {
public:
	Material(double r = 1, double g = 1, double b = 1, double d = 1.
			 , double s = 0, double sr = 1, double sg = 1, double sb = 1
			 , double re = 0, double rer = 1, double reg = 1, double reb = 1
			 , double ind = 1) 
		: color(r,g,b), diffusion_coeff(d), specularity(s), spec_color(sr,sg,sb)
		, refraction(re), refr_color(rer, reg, reb), refr_index(ind) {}
	
	Vector color;
	double diffusion_coeff;
	double specularity;
	Vector spec_color;
	double refraction; // specularity+refraction should be <= 1
	Vector refr_color;
	double refr_index;
	
};

/* Materials */

namespace Materials {
	const Material blue = Material(0,0,1);
	const Material red = Material(1,0,0);
	const Material green = Material(0,1,0);
	const Material magenta = Material(1,0,1);
	const Material yellow = Material(1,1,0);
	const Material cyan = Material(0,1,1);
	const Material mirror = Material(0,0,0,0.,1.);
	const Material yellow_mirror = Material(1,1,0,0.,0.5,1,1,0);
	const Material transparent = Material(1,1,1,0.,0.,1,1,1,1.,1,1,1,1.);
	const Material glass = Material(1,1,1,0.,0.,1,1,1,1.,1,1,1,1.5);
	const Material light_glass = Material(1,1,1,0.,0.,1,1,1,1.,1,1,1,1.4);
	const Material artistic = Material(0,1,1,0.,0.3,0,0,1,0.7,1,1,0,1.5);
	const Material mirror_glass = Material(1,1,1,0.,0.5,1,1,1,0.5,1,1,1,1.5);
}

#endif
