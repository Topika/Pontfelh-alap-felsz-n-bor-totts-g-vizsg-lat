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
    int x;
    int y;
    int z;
    preProcClass preClass;

  //public methods
  public:
    OurPoint(int x1, int y1, int z1) {
      x = x1;
      y = y1;
      z = z1;
      preClass = undef;
    }

    ~OurPoint() {}

    //getters
    const int getX() { return x; }
    const int getY() { return y; }
    const int getZ() { return z; }
    const preProcClass getPreClass() { return preClass; }

    //setters
    void setX(int x1) { x = x1; }
    void setY(int y1) { y = y1; }
    void setZ(int z1) { z = z1; }
    void setPreClass(preProcClass p1) { preClass = p1; }

    //other methods
    const double distanceFrom(const OurPoint* p2) {
      return std::sqrt( (p2->x - x) * (p2->x - x) +
                   (p2->y - y) * (p2->y - y) + 
                   (p2->z - z) * (p2->z - z) );
    }
};

#endif //OURPOINT_H

    
