#ifndef PIXEL_H
#define PIXEL_H

#include <math.h>

class Pixel {
    public:
        Pixel() : r(0), g(0), b(0) {}
        Pixel(unsigned char _r, unsigned char _g, unsigned char _b) : r(_r), g(_g), b(_b) {}
        ~Pixel() {}
        bool isWhite() {
            return r == 255 && g == 255 && b == 255;
        }
        bool isBlack() {
            return r == 0 && g == 0 && b == 0;
        }
        double getRGBDist(const Pixel& p) {
            return sqrt(pow((int) r - (int) p.r, 2) + pow((int) g - (int) p.g, 2) + pow((int) b - (int) p.b, 2));
        }

        unsigned char r, g, b;
};

#endif
