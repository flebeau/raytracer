#ifndef RAY_HPP
#define RAY_HPP

#include "vector.hpp"

class Ray {
public:
	Ray() : origin(), direction() {}
	Ray(Vector o, Vector d) : origin(o), direction(d) {}
	
	Vector origin, direction;
};

#endif
