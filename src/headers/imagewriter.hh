#ifndef __IMAGEWRITER_HH
#define __IMAGEWRITER_HH

#include <vector>
#include <iostream>

//~ #define maxX 1280
//~ #define maxY 720

class ImageWriter {
    public:
    ImageWriter(double bottomleftX_, double bottomleftY_, double toprightX_, double toprightY_) : bottomleftX(bottomleftX_), bottomleftY(bottomleftY_), toprightX(toprightX_), toprightY(toprightY_) {
        numbers = std::vector<std::vector<std::vector<int> > >(maxX, std::vector<std::vector<int> >(maxY, std::vector<int>(4,0)));
        std::cout << "Constr. done" <<std::endl;
        //~ for(int i = 0;i < maxX;++i) {
            //~ for(int j = 0;j<maxY;++j) {
                //~ for(int k = 0;k<4;++k) {
                    //~ numbers[i][j][k] = 0;
                //~ }
            //~ }
        //~ }
     }

    void addPoint(const OurPoint& newPoint);
    void writeToFile(const std::string& fileName);
    
    private:
    int bottomleftX, bottomleftY;
    int toprightX, toprightY;
    const int maxX = 1280;
    const int maxY = 720;
    //~ int numbers[maxX][maxY][4];
    std::vector<std::vector<std::vector<int> > > numbers;
    
    inline int getScaled(int val, bool x_) {
        if (x_)
            return (int)((val - bottomleftX) * (maxX-1) / (toprightX-bottomleftX));
        else
            return (int)((val - bottomleftY) * (maxY-1) / (toprightY-bottomleftY));
    }
};

#endif
