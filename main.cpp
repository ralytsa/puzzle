#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include "image.h"


 int main() {
    Image img_white, img_black;
    std::stringstream strStream;
    const int num = 6;

    // Read the file with white background
    strStream << "D:/Info/Puzzle/dogs_white_" << std::setw(4) << std::setfill('0') << num << ".bmp";
    img_white.readBMP(strStream.str());

    // Read the file with black background
    strStream.str(std::string());
    strStream << "D:/Info/Puzzle/dogs_black_" << std::setw(4) << std::setfill('0') << num << ".bmp";
    img_black.readBMP(strStream.str());

    Image imgOr = img_white.imageOR(img_black);
    Image filtered = imgOr.medianFilter();
    Image bfs = filtered.BFS(0, 0);
    Image contours = bfs.findContours(img_white);

    int size = (int) contours.pixels.size();
    std::vector <unsigned char> pixelsArr(size * 3);

    for (int i = 0; i < size; ++i) {
        pixelsArr[3 * i] = contours.pixels[i].r;
        pixelsArr[3 * i + 1] = contours.pixels[i].g;
        pixelsArr[3 * i + 2] = contours.pixels[i].b;
             
    }

    contours.displayImage(pixelsArr);

    return 0;
}
