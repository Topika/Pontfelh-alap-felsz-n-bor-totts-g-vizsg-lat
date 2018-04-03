#include <liblas/liblas.hpp>
#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include "boost/polygon/voronoi.hpp"

using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;

using namespace std;
using namespace boost::polygon;

//~ struct OurPoint {
  //~ int x;
  //~ int y;
  //~ int z;
  //~ int returns;
  //~ OurPoint (int x_, int y_, int z_, int returns_) : x(x_), y(y_), z(z_), returns(returns_) {}
//~ };
//~ 
//~ template <>
//~ struct boost::polygon::geometry_concept<OurPoint> { typedef point_concept type; };
   //~ 
//~ template <>
//~ struct boost::polygon::point_traits<OurPoint> {
  //~ typedef int coordinate_type;
    //~ 
  //~ static inline coordinate_type get(const OurPoint& point, orientation_2d orient) {
    //~ return (orient == HORIZONTAL) ? point.x : point.y;
  //~ }
//~ };

struct OurPoint {
  double x;
  double y;
  double z;
  int returns;
  OurPoint (double x_, double y_, double z_, int returns_) : x(x_), y(y_), z(z_), returns(returns_) {}
};


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
    return (orient == HORIZONTAL) ? point.x : point.y;
  }
};
}  // polygon
}  // boost


//~ class OurPoint : public point_data<double> {
    //~ public:
        //~ OurPoint(double x_, double y_, double z_, uint16_t returns_, uint16_t intensity_) : point_data<double>(x_,y_) , z(z_), returns(returns_), intensity(intensity_) {}
    //~ private:
        //~ double z;
        //~ uint16_t returns,intensity;
        //~ 
//~ };


int iterate_cells(const voronoi_diagram<double> &vd) {
  std::ofstream ofs;
  
  ofs.open("file.txt", std::ofstream::out | std::ofstream::app);
  
  for (auto it = vd.cells().begin();
       it != vd.cells().end(); ++it) {
    const voronoi_diagram<double>::cell_type &cell = *it;
    ofs << it->source_index() << std::endl;
  }
  ofs.close();
}


int main(int argc, char* argv[]) {
	std::ifstream ifs;
    
    std::vector<OurPoint> points;
    //~ std::vector<point_data<double> > points;
    
    
    cout<<__LINE__<<std::endl;
	ifs.open("sample1.las", std::ios::in | std::ios::binary);
	liblas::ReaderFactory f;
    cout<<__LINE__<<std::endl;
	liblas::Reader reader = f.CreateWithStream(ifs);
	liblas::Header const& header = reader.GetHeader();
	std::cout << "Compressed: " << header.Compressed();
    cout<<__LINE__<<std::endl;
	std::cout << "Signature: " << header.GetFileSignature() << '\n';
    cout<<__LINE__<<std::endl;
	std::cout << "Points count: " << header.GetPointRecordsCount() << '\n';
    cout<<__LINE__<<std::endl;
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
    iterate_cells(vd);
}
