#include <liblas/liblas.hpp>
#include <fstream>  // std::ifstream
#include <iostream> // std::cout
#include <functional>
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
long ROOF_LOWER_LIMIT;

preProcClass classCalculate_SSE(const OurPoint &p, const vector<OurPoint> &neighbours) {
	preProcClass resultClass = undef;

	bool isUpper = false, isLower = false;
	double SSE = 0.0;
	const double limit = 1000.0; // ~SECONDARY_TRESHOLD
	for (auto nPoint : neighbours)
	{
		long distance = nPoint.distanceFromInZ(p);
		SSE += pow(distance, 2);
		if (distance < (-1 * PRIMARY_TRESHOLD))
			isLower = true;
		else if (distance > PRIMARY_TRESHOLD)
			isUpper = true;
	}
	SSE /= neighbours.size();

	if (SSE > pow(limit, 2) && !isLower && !isUpper)
		resultClass = nonUniformSurface;
	else if (!isLower && !isUpper)
		resultClass = uniformSurface;
	else if (isLower && !isUpper)
		resultClass = lowerContour;
	else if (!isLower && isUpper)
		resultClass = upperContour;

	return resultClass;
}

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

bool checkNeighbourPoints(bool every, std::function<bool(const voronoi_diagram<double>::cell_type&, const OurPoint&)> condition,
	const voronoi_diagram<double>::cell_type &cell, const std::vector<OurPoint> &inputPoints)
{
	bool default = every ? true : false;
	if (cell.contains_point() && !cell.is_degenerate())
	{
		const voronoi_diagram<double>::edge_type *edge = cell.incident_edge();
		do
		{
			if (edge->is_primary())
			{
				const voronoi_diagram<double>::cell_type &nextCell = *edge->twin()->cell();
				if (nextCell.contains_point() && !nextCell.is_degenerate())
				{
					if (condition(nextCell, inputPoints[nextCell.source_index()])) return !default;
				}
			}
			edge = edge->next();
		} while (edge != cell.incident_edge());
	}
	return default;
}

void modifyNeighbourPoints(std::function<void(const voronoi_diagram<double>::cell_type&, OurPoint&)> fn,
	const voronoi_diagram<double>::cell_type &cell, std::vector<OurPoint> &inputPoints,
	std::function<bool(const OurPoint&)> condition = [](const OurPoint&) {return true;}, bool recursive = false)
{
	if (cell.contains_point() && !cell.is_degenerate())
	{
		const voronoi_diagram<double>::edge_type *edge = cell.incident_edge();
		do
		{
			if (edge->is_primary())
			{
				const voronoi_diagram<double>::cell_type &nextCell = *edge->twin()->cell();
				if (nextCell.contains_point() && !nextCell.is_degenerate())
				{
					OurPoint &nextPoint = inputPoints[nextCell.source_index()];
					if (condition(nextPoint))
					{
						fn(nextCell, nextPoint);
						if (recursive) modifyNeighbourPoints(fn, nextCell, inputPoints, condition, recursive);
					}
				}
			}
			edge = edge->next();
		} while (edge != cell.incident_edge());
	}
}

void expandRoof(const voronoi_diagram<double>::cell_type &startCell, std::vector<OurPoint> &inputPoints, const long &limit) {
	auto condition = [&limit](const OurPoint& point)->bool {
		return (point.getPreClass() == uniformSurface && point.getZ() > limit);
	};
	auto expandRoofLogic = [](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
		point.setPreClass(roof);
	};

	modifyNeighbourPoints(expandRoofLogic, startCell, inputPoints, condition, true);
}

void voronoiCellIteration(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints,
	std::function<bool(const OurPoint &)> startPointCondition, std::function<void(const voronoi_diagram<double>::cell_type&, OurPoint&)> task)
{
	for (auto it = vd.cells().begin(); it != vd.cells().end(); ++it)
	{
		const voronoi_diagram<double>::cell_type &startCell = *it;
		if (startCell.contains_point() && !startCell.is_degenerate())
		{
			OurPoint &startPoint = inputPoints[startCell.source_index()];
			if (startPointCondition(startPoint)) task(startCell, startPoint);
		}
	}
}

void determineRoof(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints) {
	auto condition = [](const OurPoint &point)->bool {
		return (point.getPreClass() == upperContour && point.getZ() > ROOF_LOWER_LIMIT);
	};
	auto task = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
		auto roofNeighbourLogic = [](const voronoi_diagram<double>::cell_type &cell, const OurPoint &point)->bool {
			return (point.getPreClass() == roof);
		};

		expandRoof(cell, inputPoints, ROOF_LOWER_LIMIT);
		if (checkNeighbourPoints(false, roofNeighbourLogic, cell, inputPoints))
			point.isRoofContour = true;
	};

	voronoiCellIteration(vd, inputPoints, condition, task);
}

void mergeRoofContour(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints) {
	auto condition = [](const OurPoint &point)->bool {
		return point.isRoofContour;
	};
	auto task = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
		auto roofContourLogic = [](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
			if (point.getPreClass() == upperContour)
			{
				point.isRoofContour = true;
				point.setPreClass(roof);
			}
		};

		point.setPreClass(roof);
		modifyNeighbourPoints(roofContourLogic, cell, inputPoints);
	};

	voronoiCellIteration(vd, inputPoints, condition, task);
}

void voronoiOpeningErosion(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints) {
	auto condition = [](const OurPoint &point)->bool {
		return point.getPreClass() == roof;
	};

	auto task = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
		auto openingErosionLogic = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, const OurPoint &point)->bool {
			auto kernelLogic = [](const voronoi_diagram<double>::cell_type &cell, const OurPoint &point)->bool {
				return (point.getPreClass() != roof);
			};
			return (point.getPreClass() != roof || checkNeighbourPoints(false, kernelLogic, cell, inputPoints));
		};

		if (checkNeighbourPoints(true, openingErosionLogic, cell, inputPoints))
			point.isInnerBuilding = true;
	};

	voronoiCellIteration(vd, inputPoints, condition, task);
}

void voronoiClosingErosion(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints) {
	auto condition = [](const OurPoint &point)->bool {
		return point.isOuterBuilding;
	};

	auto task = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
		auto closingErosionLogic = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, const OurPoint &point)->bool {
			auto kernelLogic = [](const voronoi_diagram<double>::cell_type &cell, const OurPoint &point)->bool {
				return (!point.isOuterBuilding);
			};
			return ((!point.isOuterBuilding) || checkNeighbourPoints(false, kernelLogic, cell, inputPoints));
		};

		if (checkNeighbourPoints(true, closingErosionLogic, cell, inputPoints))
			point.isBuilding = true;
	};

	voronoiCellIteration(vd, inputPoints, condition, task);
}

void voronoiOpeningDilatation(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints) {
	auto condition = [](const OurPoint &point)->bool {
		return point.isInnerBuilding;
	};

	auto task = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
		auto openingDilatationLogic = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
			auto kernelLogic = [](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
				point.isBuilding = true;
			};
			point.isBuilding = true;
			modifyNeighbourPoints(kernelLogic, cell, inputPoints);
		};

		point.isBuilding = true;
		modifyNeighbourPoints(openingDilatationLogic, cell, inputPoints);
	};

	voronoiCellIteration(vd, inputPoints, condition, task);
}

void voronoiClosingDilatation(const voronoi_diagram<double> &vd, std::vector<OurPoint> &inputPoints) {
	auto condition = [](const OurPoint &point)->bool {
		return point.isBuilding;
	};

	auto task = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
		auto closingDilatationLogic = [&inputPoints](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
			auto kernelLogic = [](const voronoi_diagram<double>::cell_type &cell, OurPoint &point) {
				point.isOuterBuilding = true;
			};
			point.isOuterBuilding = true;
			modifyNeighbourPoints(kernelLogic, cell, inputPoints);
		};

		point.isOuterBuilding = true;
		modifyNeighbourPoints(closingDilatationLogic, cell, inputPoints);
	};

	voronoiCellIteration(vd, inputPoints, condition, task);
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
		preProcClass currentPointClass = classCalculate_SSE(currentPoint, getNeighboursOfPoint(cell, inputPoints));
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

int translatePreProcClass_lasview(preProcClass ppc, bool isRoofContour = false)
{
	if (isRoofContour) return 5;
	switch (ppc)
	{
	case upperContour: return 6;
	case lowerContour: return 3;
	case uniformSurface: return 2;
	case nonUniformSurface: return 9;
	case roof: return 12;
	default: return 0;
	}
}

int translatePreProcClass_binary(bool condition)
{
	return condition ? 9 : 12;
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
	ROOF_LOWER_LIMIT = (long)((header.GetMinZ() + (header.GetMaxZ() - header.GetMinZ()) / 4.0) * SCALE_Z);

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
	determineRoof(vd, points);
	for (int i = 0; i < 5; ++i)
		mergeRoofContour(vd, points); // recursion crashed, maybe stack overflow?

	// opening
	voronoiOpeningErosion(vd, points);
	voronoiOpeningDilatation(vd, points);
	// closing
	voronoiClosingDilatation(vd, points);
	voronoiClosingErosion(vd, points);


	std::ofstream ofs("custom_classes_roof.las", ios::out | ios::binary);
	liblas::Writer writer(ofs, header);

	for (auto point : points)
	{
		liblas::Point output(&header);
		output.SetRawX(point.getX());
		output.SetRawY(point.getY());
		output.SetRawZ(point.getZ());
		liblas::Classification customClass;
		// customClass.SetClass(translatePreProcClass_lasview(point.getPreClass(), point.isRoofContour));
		//customClass.SetClass(translatePreProcClass_binary(point.getPreClass() == roof));
		customClass.SetClass(translatePreProcClass_binary(point.isBuilding));
		output.SetClassification(customClass);
		writer.WritePoint(output);
	}

	return 0;
}
