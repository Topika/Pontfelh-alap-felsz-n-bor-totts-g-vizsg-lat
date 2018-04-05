#include <liblas/liblas.hpp>
#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include "boost/polygon/voronoi.hpp"
#include "headers/OurPoint.h"

using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;

using namespace std;
using namespace boost::polygon;

namespace boost {
namespace polygon {

  template <>
  struct geometry_concept<OurPoint> {
    typedef point_concept type;
  };

  template <>
  struct point_traits<OurPoint> {
    typedef int coordinate_type;

    static inline coordinate_type get(
        const OurPoint& point, orientation_2d orient) {
      return (orient == HORIZONTAL) ? point.getX() : point.getY();
    }
  };
}  // polygon
}  // boost

const double s1 = 2000.0; //?

/*
-- Get the examined point, and its neighbours. Calculate the class
-- of the point with the threshold s1 
*/
preProcClass classCalculate(const OurPoint &p, const vector<OurPoint> &neighbours) {
  preProcClass resultClass = undef;
    
  bool isInside, isUpper, isLower = false; 
  for (auto nPoint : neighbours)
  {
    double distance = nPoint.distanceFromInZ(p);
    if (distance < (-1 * s1))
       isLower = true;
    else if (distance > s1)
      isUpper = true;
    else
      isInside = true;
  }

  if (isInside && !isLower && !isUpper)
    resultClass = uniformSurface; // it can be nonuniformSurface too, TODO
  else if (isInside && isLower && !isUpper)
    resultClass = lowerContour;
  else if (isInside && !isLower && isUpper)
     resultClass = upperContour;

  return resultClass;
}

vector<OurPoint> getNeighboursOfPoint(const voronoi_diagram<double>::cell_type &point, const std::vector<OurPoint> &inputPoints) {
  vector<OurPoint> result;
  if (point.contains_point() && !point.is_degenerate())
  {
    const voronoi_diagram<double>::edge_type *edge = point.incident_edge();
    do
    {
      if (edge->is_primary())
      {
        auto index = edge->twin()->cell()->source_index();
        result.push_back(inputPoints[index]);
      }
      edge = edge->next();
    } while (edge != point.incident_edge());
  }
  return result;
}


void iterateCells(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints) {
  std::ofstream ofs;
  ofs.open("file.txt", std::ofstream::out | std::ofstream::app);
  int upperCnt = 0, lowerCnt = 0, surfaceCnt = 0, undefCnt = 0;
  
  for (auto it = vd.cells().begin(); it != vd.cells().end(); ++it)
  {
    const voronoi_diagram<double>::cell_type &cell = *it;
    //ofs << it->source_index() << std::endl;
  
    OurPoint& currentPoint = inputPoints[cell.source_index()];
    preProcClass actualPointClass = classCalculate(currentPoint, getNeighboursOfPoint(cell, inputPoints));
    currentPoint.setPreClass(actualPointClass);
    
    switch (currentPoint.getPreClass())
    {
    case upperContour: ++upperCnt; break;
    case lowerContour: ++lowerCnt; break;
    case uniformSurface:
    case nonUniformSurface: ++surfaceCnt; break;
    default: ++undefCnt; break;
    }
  }
  ofs << "Upper: " << upperCnt << std::endl
	  << "Lower: " << lowerCnt << std::endl
	  << "Surface: " << surfaceCnt << std::endl
	  << "Other: " << undefCnt << std::endl;
  ofs.close();
}


int main(int argc, char* argv[]) {
	std::ifstream ifs;
    
    std::vector<OurPoint> points;    
    
    cout<<__LINE__<<std::endl;
	ifs.open("../data/sample1.las", std::ios::in | std::ios::binary);
	liblas::ReaderFactory f;
	liblas::Reader reader = f.CreateWithStream(ifs);
	liblas::Header const& header = reader.GetHeader();
	std::cout << "Compressed: " << header.Compressed() << '\n';
	//std::cout << "Signature: " << header.GetFileSignature() << '\n';
	std::cout << "Points count: " << header.GetPointRecordsCount() << '\n';
    
	//~ The voronoi diagram
    voronoi_diagram<double> vd;
    
	while (reader.ReadNextPoint())
	{
		liblas::Point const& p = reader.GetPoint();
        //~ points.push_back(point_data<double>(p.GetX(),p.GetY()));
        points.push_back(OurPoint(p.GetRawX(),p.GetRawY(),p.GetRawZ(),p.GetReturnNumber()));
        //~ boost::polygon::insert<OurPoint,voronoi_diagram<double> >(p, &vd);

		//~ std::cout << p.GetX() << ", " << p.GetY() << ", " << p.GetZ() << "\n";
	}
    construct_voronoi(points.begin(), points.end(), &vd);
    
    std::cout << "Num of cells: " << vd.num_cells() << std::endl;
    std::cout << "Num of edges: " << vd.num_edges() << std::endl;
    
	iterateCells(vd, points);
}
