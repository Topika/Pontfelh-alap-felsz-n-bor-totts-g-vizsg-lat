//~ #include <iostream>
#include <fstream>  // std::ofstream
#include "headers/OurPoint.h"
#include "headers/imagewriter.hh"
#include <opencv2/highgui.hpp>


using namespace cv;
using namespace std;

static Vec3b toColor(int x) {
    switch(x) {
        case 0: return Vec3b(42,42,165); //talaj, barna
        case 1:
            return Vec3b(0,255,0); // növény, zöld
        case 2:
            return Vec3b(0,0,255); // épület, vörös
        case 3:
            return Vec3b(32,32,32); // út, fekete
        default:
            return Vec3b(255,255,255);
        
        //~ case 0: return Vec3b(255,0,0);
        //~ case 1: return Vec3b(0,255,0);
        //~ case 2: return Vec3b(0,0,255);
        //~ case 3: return Vec3b(128,128,255);
        //~ default: return Vec3b(0,0,0);
    }
    
}

void ImageWriter::addPoint(const OurPoint& newPoint) {
    int32_t x = newPoint.getX();
    int32_t y = newPoint.getY();
    preProcClass realClass = newPoint.getPreClass();
        //~ cout << (int)(x * scaleX) << " " <<(int)( y * scaleY )<< endl;
    int transformed = -1;
    if(realClass == uniformSurface) transformed = 0;
    else if(realClass == nonUniformSurface || realClass == lowerContour || realClass == upperContour) transformed = 1;
    else if(realClass == building) transformed = 2;
    else if(realClass == road) transformed = 3;
    //~ cout << getScaled(x,true) << " " << getScaled(y,false) << " " << transformed << endl;
    if(transformed != -1 && transformed != 2)
        ++numbers[getScaled(x,true)][getScaled(y,false)][transformed];
    else if (transformed == 2)
        numbers[getScaled(x,true)][getScaled(y,false)][transformed] += 1;
    //~ cout << "end" << endl;
    //~ short numbers[1280][720][4];
}

void ImageWriter::writeToFile(const std::string& fileName) {
    Mat mat(maxY,maxX,CV_8UC3);
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
