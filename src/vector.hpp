#ifndef VECTOR_HPP
#define VECTOR_HPP

class Vector {
public:
	/* Constructors */
	Vector() : x(0), y(0), z(0) {}
	Vector(double x1, double y1, double z1) : x(x1), y(y1), z(z1) {}
	
	/* Operations */
	Vector operator*(const double &alpha) const;
	Vector operator+(const Vector &v) const;	
	Vector operator-(const Vector &v) const;	
	Vector operator-() const;

	double sp(const Vector &v) const; // Scalar product
	Vector vp(const Vector &v) const; // Vector product
	double snorm() const; // Square of the norm
	double norm() const;
	Vector normalize() const;
	
	/* Coordinates */
	double x,y,z;
};

#endif
