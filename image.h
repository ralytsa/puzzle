#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <string>

#include "pixel.h"
#include "point.h"

const int MAX_BLACK = 5; // Maximal value of r | g | b component to make a pixel black in binarize()
const int NEIGHBOURS_CNT = 8; // Possible neighbours count
const double MIN_RGB_DIST = 120.0;
const int neighbours[8][2] = { { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 } ,{ 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 } };

class Image {
    public:
        Image() : width(0), height(0), pixels(0) {}
        Image(const int &_w, const  int &_h, const std::vector <Pixel> _p);
        Image(const Image& img) : width(img.width), height(img.height), pixels(img.pixels) {}
        Image& operator = (const Image& img);
        Pixel& operator [] (const unsigned int &ind) { return pixels[ind]; }
        ~Image() {}
        
       void readBMP(const std::string filename);
       void displayImage(std::vector <unsigned char> pixelsArr);

       Image addWhitePadding();

       Image imageOR(Image& img);
       Image medianFilter();
       Image BFS(int x, int y);
       Image findContours(Image& image_white);
       Image convexHull();

       int width, height;
       std::vector <Pixel> pixels;
       std::vector < std::vector <Point> > contours;
};

#endif
