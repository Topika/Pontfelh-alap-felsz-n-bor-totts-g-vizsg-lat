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

long SCALE_Z; // current las file scale value
long PRIMARY_TRESHOLD; // surface/contour threshold
long SECONDARY_TRESHOLD;   // uniform/non-uniform surface threshold

/*
-- Get the examined point, and its neighbours. Calculate the class
-- of the point with the PRIMARY_TRESHOLD
*/
preProcClass classCalculate(const OurPoint &p, const vector<OurPoint> &neighbours) {
	preProcClass resultClass = undef;
    
	bool isInAndClose = false, isInAndFar = false, isUpper = false, isLower = false;
	for (auto nPoint : neighbours)
	{
		long distance = nPoint.distanceFromInZ(p);
		if (distance < (-1 * PRIMARY_TRESHOLD))
			isLower = true;
		else if (distance > PRIMARY_TRESHOLD)
			isUpper = true;
		else if (distance >= (-1 * SECONDARY_TRESHOLD) && distance <= SECONDARY_TRESHOLD)
			isInAndClose = true;
		else
			isInAndFar = true;
	}

	if (isInAndFar && !isLower && !isUpper)
		resultClass = nonUniformSurface;
	else if (isInAndClose && !isInAndFar && !isLower && !isUpper)
		resultClass = uniformSurface;
	else if ((isInAndFar || isInAndClose) && isLower && !isUpper)
		resultClass = lowerContour;
	else if ((isInAndFar || isInAndClose) && !isLower && isUpper)
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
	//std::ofstream ofs;
	//ofs.open("file.txt", std::ofstream::out | std::ofstream::app);
	int upperCnt = 0, lowerCnt = 0, uniformCnt = 0, nonUniformCnt = 0,undefCnt = 0;

	for (auto it = vd.cells().begin(); it != vd.cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type &cell = *it;
		//ofs << it->source_index() << std::endl;

		OurPoint& currentPoint = inputPoints[cell.source_index()];
		preProcClass currentPointClass = classCalculate(currentPoint, getNeighboursOfPoint(cell, inputPoints));
		currentPoint.setPreClass(currentPointClass);

		switch (currentPoint.getPreClass())
		{
			case upperContour: ++upperCnt; break;
			case lowerContour: ++lowerCnt; break;
			case uniformSurface: ++uniformCnt; break;
			case nonUniformSurface: ++nonUniformCnt; break;
			default: ++undefCnt; break;
		}
	}
	std::cout << "The calculated classes are:" << std::endl 
		<< "Upper contour: " << upperCnt << std::endl
		<< "Lower contour: " << lowerCnt << std::endl
		<< "Uniform surface: " << uniformCnt << std::endl
		<< "Non-uniform surface: " << nonUniformCnt << std::endl
		<< "Other: " << undefCnt << std::endl;
	/*ofs << "Upper: " << upperCnt << std::endl
		<< "Lower: " << lowerCnt << std::endl
		<< "Surface: " << surfaceCnt << std::endl
		<< "Other: " << undefCnt << std::endl;
	ofs.close();*/
}

int main(int argc, char* argv[]) {
	std::ifstream ifs;

	ifs.open("../data/sample1.las", std::ios::in | std::ios::binary);
	liblas::ReaderFactory f;
	liblas::Reader reader = f.CreateWithStream(ifs);
	liblas::Header const& header = reader.GetHeader();
	std::cout << std::endl << "1) Read 'sample1.las', which has the next information:" << std::endl;
	std::cout << "Compressed: " << header.Compressed() << '\n';
	//std::cout << "Signature: " << header.GetFileSignature() << '\n';
	std::cout << "Points count: " << header.GetPointRecordsCount() << '\n';
    std::cout << std::endl << "--------------------------------------------------------" << std::endl;

	SCALE_Z = (long)(1.0 / header.GetScaleZ());
	PRIMARY_TRESHOLD = 2 * SCALE_Z;
	SECONDARY_TRESHOLD = PRIMARY_TRESHOLD / 2;

	std::vector<OurPoint> points;
	points.reserve(header.GetPointRecordsCount());
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

	std::cout << std::endl << "2) Finished the calculation of voronoi diagram. The result is:" << std::endl; 
	std::cout << "Number of cells: " << vd.num_cells() << std::endl;
	std::cout << "Number of edges: " << vd.num_edges() << std::endl;
    std::cout << std::endl << "--------------------------------------------------------" << std::endl;
    std::cout << "3) Start to examine all the points and their neighbours, to calculate the first segmentation to surfaces and contours..." << std::endl;

	iterateCells(vd, points);


	std::ofstream ofs("custom_classes.las", ios::out | ios::binary);
	liblas::Writer writer(ofs, header);

	for (auto point : points)
	{
		liblas::Point output(&header);
		output.SetRawX(point.getX());
		output.SetRawY(point.getY());
		output.SetRawZ(point.getZ());
		liblas::Classification customClass;
		customClass.SetClass(point.getPreClass());
		// customClass.SetClass(point.getPreClass() == undef ? 9 : 12); // separating undef category only
		output.SetClassification(customClass);
		writer.WritePoint(output);
	}
	
	return 0;
}

