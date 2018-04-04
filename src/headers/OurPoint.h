#ifndef OURPOINT_H
#define OURPOINT_H

#include <iostream>
#include <cmath>

enum preProcClass {
  undef,
  upperContour, lowerContour,
  uniformSurface, nonUniformSurface
};

class OurPoint {
  
  //private datas
  private:
    double x;
    double y;
    double z;
	int returns;
    preProcClass preClass;

  //public methods
  public:
    OurPoint (double x_, double y_, double z_, int returns_) : 
      x(x_), y(y_), z(z_), preClass(undef), returns(returns_) {}

    ~OurPoint() {}

    //getters
    double getX() const { return x; }
    double getY() const { return y; }
    double getZ() const { return z; }
	int getReturns() const { return returns; }
    preProcClass getPreClass() const { return preClass; }

    //setters
    void setX(double x1) { x = x1; }
    void setY(double y1) { y = y1; }
    void setZ(double z1) { z = z1; }
	void setReturns(int ret) { returns = ret; }
    void setPreClass(preProcClass p1) { preClass = p1; }

    //other methods
    double distanceFromInZ(const OurPoint& p2) {
      return (p2.z - z);
    }
};

#endif //OURPOINT_H

    
