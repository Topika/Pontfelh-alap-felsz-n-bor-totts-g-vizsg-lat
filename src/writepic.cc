//~ #include <iostream>
#include <fstream>  // std::ofstream
#include "headers/OurPoint.h"
#include "headers/imagewriter.hh"
#include <opencv2/highgui.hpp>


using namespace cv;
using namespace std;

static Vec3b toColor(int x) {
    switch(x) {
        case 0: return Vec3b(255,0,0);
        case 1: return Vec3b(0,255,0);
        case 2: return Vec3b(0,0,255);
        case 3: return Vec3b(128,128,255);
        default: return Vec3b(0,0,0);
    }
    
}
//~ class ImageWriter {
    //~ public:
    //~ ImageWriter() {}
    //~ void init(int xSize,int ySize, int t);
    //~ void addPoint(const OurPoint& newPoint);
    //~ void writeToFile(const std::string& fileName);
    //~ 
    //~ private:
    //~ Mat mat;
//~ } 

void ImageWriter::addPoint(const OurPoint& newPoint) {
    int32_t x = newPoint.getX();
    int32_t y = newPoint.getY();
    preProcClass realClass = newPoint.getPreClass();
        //~ cout << (int)(x * scaleX) << " " <<(int)( y * scaleY )<< endl;

    ++numbers[getScaled(x,true)][getScaled(y,false)][realClass];
    //~ short numbers[1280][720][4];
}

void ImageWriter::writeToFile(const std::string& fileName) {
    Mat mat(maxX,maxY,CV_8UC3);
    for(int i = 0; i < maxX; ++i) {
        for(int j = 0; j < maxY; ++j) {
            int max = numbers[i][j][0], maxid = 0;
            for(int k = 1; k < 4; ++k) {
                if (max < numbers[i][j][k]) {
                    maxid = k;
                    max = numbers[i][j][k];
                }
            }
            mat.at<Vec3b>(Point(i,j)) = toColor(maxid);
        }
    }
    imwrite( fileName, mat );
}
