#ifndef OURPOINTCLOUD_H
#define OURPOINTCLOUD_H

#include "OurPoint.h"
#include <iostream>
#include <liblas/liblas.hpp>
#include <fstream>
#include <cmath>
#include <vector>

class OurPointCloud {
  
  //private datas
  private:
    std::vector<OurPoint*> points;

  //public methods
  public:
    OurPointCloud(const std::string lasFile) {
      
      const char* file = lasFile.c_str();
      std::ifstream ifs;
      ifs.open(file, std::ios::in | std::ios::binary);
      
      liblas::ReaderFactory f;
      liblas::Reader reader = f.CreateWithStream(ifs);

      while (reader.ReadNextPoint()) {
        liblas::Point const& p = reader.GetPoint();
        points.push_back(new OurPoint(p.GetX(), p.GetY(), p.GetZ()));
      }
    }

    ~OurPointCloud() {}

    //other methods
    void calculateVoronoi () {}

    preProcClass OnePointSubClassCalculate(const OurPoint p) {}
};

#endif //OURPOINTCLOUD_H

    
