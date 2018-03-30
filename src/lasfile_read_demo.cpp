#include <liblas/liblas.hpp>
#include <fstream>
#include <iostream>

#include <cstdlib>

int main()
{
	const char *file = "../data/sample1.laz";
	std::ifstream ifs;
	ifs.open(file, std::ios::in | std::ios::binary);

	liblas::ReaderFactory f;
	liblas::Reader reader = f.CreateWithStream(ifs);

	liblas::Header const& header = reader.GetHeader();
	std::cout << "Points count: " << header.GetPointRecordsCount() << std::endl; // Points count: 24973

	// processing
	//while (reader.ReadNextPoint())
	//{
		//liblas::Point const& p = reader.GetPoint();
		//std::cout << p.GetX() << ", " << p.GetY() << ", " << p.GetZ() << std::endl;
	//}

	system("pause");
}
