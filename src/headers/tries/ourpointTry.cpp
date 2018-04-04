#include "..\OurPoint.h"
#include <fstream>
#include <iostream>

#include <cstdlib>

int main()
{
  OurPoint* a = new OurPoint(1,2,3);
  std::cout << a->getX() << " " << a->getY() << " " << a->getZ() << " " << std::endl;
  std::cout << a->distanceFrom(new OurPoint(10,11,19)) << std::endl;
  a->setX(30); a->setY(20); a->setZ(51); a->setPreClass(upperContour);
  std::cout << a->getX() << " " << a->getY() << " " << a->getZ() << " " << std::endl;
  std::cout << a->distanceFrom(new OurPoint(10,11,19)) << std::endl;

  return 0;
}
