#ifndef OURPOINT_H
#define OURPOINT_H

#include <iostream>
#include <cmath>

enum preProcClass {
	undef=-1,
	upperContour = 0, lowerContour = 1,
	uniformSurface = 2, nonUniformSurface = 3
, roof //temporarrily added
};

class OurPoint {
	//private datas
	private:
		long x;
		long y;
		long z;
		int returns;
		preProcClass preClass;

	//public methods
	public:
	OurPoint (long x_, long y_, long z_, int returns_) : 
		x(x_), y(y_), z(z_), preClass(undef), returns(returns_) {}

	~OurPoint() {}

	//getters
	long getX() const { return x; }
	long getY() const { return y; }
	long getZ() const { return z; }
		int getReturns() const { return returns; }
	preProcClass getPreClass() const { return preClass; }

	//setters
	void setX(long x1) { x = x1; }
	void setY(long y1) { y = y1; }
	void setZ(long z1) { z = z1; }
	void setReturns(int ret) { returns = ret; }
	void setPreClass(preProcClass p1) { preClass = p1; }

	//other methods
	long distanceFromInZ(const OurPoint& p2) const {
		return (p2.z - z);
	}

	bool isRoofContour = false;
	bool isInnerBuilding = false;
	bool isOuterBuilding = false;
	bool isBuilding = false;
};

#endif //OURPOINT_H

