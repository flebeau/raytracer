#ifndef VECTOR_HPP
#define VECTOR_HPP

class Vector {
public:
	/* Constructors */
	Vector() : x(0), y(0), z(0) {}
	Vector(double x1, double y1, double z1) : x(x1), y(y1), z(z1) {}
	
	/* Operations */
	Vector operator*(const double &alpha) const; // Product by a constant
	Vector operator*(const Vector &v) const; // Coordinate product
	Vector operator+(const Vector &v) const;
	Vector operator-(const Vector &v) const;	
	Vector operator-() const;
	bool operator==(const Vector &v) const {
		return (v.x == x && v.y == y && v.z == z);
	}

	double sp(const Vector &v) const; // Scalar product
	Vector vp(const Vector &v) const; // Vector product
	double snorm() const; // Square of the norm
	double norm() const;
	Vector normalize() const; // Return the normalized vector
	
	// Convert the current vector written in a coordinate system expressed in the
	// canonical system to the canonical system
	void convertCoordinateSystem(Vector u, Vector v, Vector w);
	
	/* Coordinates */
	double x,y,z;
};

Vector operator*(double alpha, const Vector &v); // Reflexive version (to multiply by a constant on the right)

Vector generateUniformRandomVector(); // Returns a uniform random vector in the unit half-sphere
double getUniformNumber(); // Returns a random vector between 0 and 1

#endif
