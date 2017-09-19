#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include <SDL.h>
#include <SDL_image.h>
#include "image.h"


Image::Image(const int &_w, const int &_h, const std::vector <Pixel> _p) : width(_w), height(_h) {
    int size = (int) _p.size() ;
    pixels.resize(size);
    for (int i = 0; i < size; ++i) {
        pixels[i] = _p[i];
    }
}


Image& Image::operator=(const Image& img) {
    if (this != &img) {
        width = img.width;
        height = img.height;
        pixels = img.pixels;
    }

    return *this;
}


void Image::readBMP(const std::string filename) {
    std::ifstream ifs;
    ifs.open(filename.c_str(), std::ios::binary);

    try {
        if (ifs.fail()) {
            throw("Can't open input file");
        }
        
        // File Header
        char bmpFileHeader[14];
        ifs.read(bmpFileHeader, 14);
        
        if (bmpFileHeader[0] != 'B' || bmpFileHeader[1] != 'M') {
            std::cerr << "The file info header is different than 'BM'. \n";
        }
        
        // Array Offset 
        unsigned int pixelArrayOffset;
        memcpy(&pixelArrayOffset, bmpFileHeader + 10, 4);
        
        // File Info Header
        char bmpInfoHeader[40];
        ifs.read(bmpInfoHeader, 40);
        
        memcpy(&width, bmpInfoHeader + 4, 4);
        memcpy(&height, bmpInfoHeader + 8, 4);
        int bitsPerPixel;
        memcpy(&bitsPerPixel, bmpInfoHeader + 14, 4);

        if (bitsPerPixel != 24) {
            std::cerr << "Expecting 24bpp files. \n";
        }
        
        int compressionMethod;
        memcpy(&compressionMethod, bmpInfoHeader +  16, 4);

        if (compressionMethod != 0) {
            std::cerr << "There is a compression method used. \n";
        }

        int newWidth = 3 * width;
        int size = width * height;
        pixels.resize(size);

        int ind = size - 1;
        for (int i = (height - 1); i >= 0; --i) {
            unsigned char * pixelsArr = new unsigned char[newWidth];
            ifs.read((char *) pixelsArr, newWidth);

            for (int j = (newWidth - 1); j >= 0; j -= 3) {
                pixels[ind--] = Pixel(pixelsArr[j - 2], pixelsArr[j - 1], pixelsArr[j]);
            }
            delete[] pixelsArr;
        }
        
        ifs.close();
        
    } catch (const char* err) {
        std::cerr << err;
        ifs.close();
    }
}


void Image::displayImage(std::vector <unsigned char> pixelsArr) {
    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *bitmapTex = NULL;
    SDL_Surface* surfTemp = NULL;

    if ((surfTemp = SDL_CreateRGBSurfaceFrom(&pixelsArr[0], width, height, 24, 3 * width, 0, 0, 0, 0)) == NULL) {
        std::cerr << "Surface not created!";

        SDL_FreeSurface(surfTemp);

        return;
    }

    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("", 50, 150, width, height, 0);

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    bitmapTex = SDL_CreateTextureFromSurface(renderer, surfTemp);
    SDL_FreeSurface(surfTemp);

    while (1) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(bitmapTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    SDL_Quit();

}


Image Image::addWhitePadding() {
    // Add white border to the original image
    std::vector <Pixel> paddedImagePixels((width + 2) * (height + 2), Pixel(255, 255, 255));
    Image paddedImage = Image(width + 2, height + 2, paddedImagePixels);
    for (int i = 1; i <= height; ++i) {
        for (int j = 1; j <= width; ++j) {
            paddedImage[i * (width + 2) + j] = pixels[(i - 1) * width + j - 1];
        }
    }

    return paddedImage;
}


// Try to find the puzzle pieces by taking only the pixels that have similar rgb components
Image Image::imageOR(Image& img) {
    int thisSize = (int)pixels.size();
    int imgSize = (int)img.pixels.size();
    if (thisSize != imgSize) {
        return *this;
    }

    std::vector <Pixel> newPixels(thisSize, Pixel(0, 0, 0));
    for (int i = 0; i < thisSize; ++i) {
        if (pixels[i].getRGBDist(img[i]) > MIN_RGB_DIST) {
            newPixels[i] = Pixel(255, 255, 255);
        }
    }

    return Image(width, height, newPixels);
}


Image Image::medianFilter() {
    Image filtered(width, height, pixels);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double average = 0;
            int cnt = 0;

            for (int k = 0; k < NEIGHBOURS_CNT; ++k) {
                int newH = i + neighbours[k][0];
                int newW = j + neighbours[k][1];
                if (newH >= 0 && newH < height && newW >= 0 && newW < width) {
                    average += pixels[newH * width + newW].r;
                    ++cnt;
                }
            }
            average /= cnt;
            filtered[i * width + j] = Pixel((unsigned char) average, (unsigned char) average, (unsigned char) average);
        }
    }

    return filtered;
}

Image Image::BFS(int x, int y) {
    int size = (int)pixels.size();
    std::vector <Pixel> newPixels(size, Pixel(0, 0, 0));
    std::vector <bool> visited(size, 0);
    std::queue <Point> q;

    // Find a white pixel to start the BFS from
    int foundWhite = false;
    for (int i = 0; i < height; ++i) {
        if (foundWhite) break;
        for (int j = 0; j < width; ++j) {
            if (foundWhite) break;
            if (pixels[i * width + j].isWhite()) {
                x = j;
                y = i;
                foundWhite = true;
            }
        }
    }

    visited[y * width + x] = 1;
    q.push(Point(x, y));

    while (!q.empty()) {
        Point p = q.front();
        q.pop();

        for (int i = 0; i < NEIGHBOURS_CNT; ++i) {
            int y2 = p.y + neighbours[i][0];
            int x2 = p.x + neighbours[i][1];
            int pos = y2 * width + x2;

            if (y2 >= 0 && y2 < height && x2 >= 0 && x2 < width && !visited[pos] && pixels[pos].isWhite()) {
                visited[pos] = 1;
                newPixels[pos] = Pixel(255, 255, 255);

                q.push(Point(x2, y2));
            }
        }
    }

    return Image(width, height, newPixels);
}

Image Image::findContours(Image& image_white) {
    bool inContour = false;
    Image paddedImage = addWhitePadding();
    std::vector <Pixel> newPixels((width + 2) * (height + 2), Pixel(255, 255, 255));

    for (int i = 0; i < (height + 2); ++i) {
        for (int j = 0; j < (width + 2); ++j) {
            int pos = i * (width + 2) + j;
            if (newPixels[pos].isBlack() && !inContour) {
                inContour = true;
            }
            else if (paddedImage[pos].isBlack() && inContour) {
                continue;
            }
            else if (paddedImage[pos].isWhite() && inContour) {
                inContour = false;
            }
            else if (paddedImage[pos].isBlack() && !inContour) {
                std::vector <Point> contour;
                newPixels[pos] = Pixel(0, 0, 0);
                contour.push_back(Point(j, i));
                int startW = j;
                int startH = i;
                int curW = startW;
                int curH = startH;
                int checkLocation = 0;
                int nextToCheck;
                int checkW;
                int checkH;
                int counter = 0;

                int moves[8] = {7, 7, 1, 1, 3, 3, 5, 5};

                while (true) {
                    checkH = curH + neighbours[checkLocation][0];
                    checkW = curW + neighbours[checkLocation][1];
                    nextToCheck = moves[checkLocation];
                    if (checkH >= 0 && checkH < (height + 2) && checkW >= 0 && checkW < (width + 2) && paddedImage[checkH * (width + 2) + checkW].isBlack()) {
                        if (checkH == startH && checkW == startW) {
                            contours.push_back(contour);
                            break;
                        }

                        checkLocation = nextToCheck;
                        curW = checkW;
                        curH = checkH;
                        counter = 0;
                        newPixels[checkH * (width + 2) + checkW] = Pixel(0, 0, 0);
                        contour.push_back(Point(checkW, checkH));
                    }
                    else {
                        checkLocation = ((1 + checkLocation) % 8);
                        // Single black pixel
                        if (counter > 8) {
                            counter = 0;
                            contours.push_back(contour);
                            break;
                        }
                        else {
                            ++counter;
                        }
                    }
                }
            }
        }
    }

    // Remove white border
   int size = height * width;
    std::vector <Pixel> retPixels = std::vector<Pixel> (size, Pixel(255, 255, 255));

    for (int i = 1; i <= height; ++i) {
        for (int j = 1; j <= width; ++j) {
            retPixels[(i - 1) * width + j - 1] = newPixels[i * (width + 2) + j];
        }
    }


    for (int ind = 0; ind < (int)contours.size(); ++ind) {
        std::vector <bool> visited(size, 0);
        std::vector<Point> s;
        s.push_back(contours[ind][0]);
        retPixels[contours[ind][0].y * width + contours[ind][0].x] = image_white[contours[ind][0].y * width + contours[ind][0].x];
        while (!s.empty()) {
            Point p = s.back();
            s.pop_back();

            if (!visited[p.y * width + p.x]) {
                visited[p.y * width + p.x] = 1;
                retPixels[p.y * width + p.x] = image_white[p.y * width + p.x];
                for (int i = 0; i < 8; ++i) {
                    int newH = p.y + neighbours[i][0];
                    int newW = p.x + neighbours[i][1];
                    if (newH >= 0 && newH < height && newW >= 0 && newW < width && pixels[newH * width + newW].isBlack()) {
                        s.push_back(Point(newW, newH));
                    }
                }
            }
        }
    }

    return Image(width, height, retPixels);
}

Image Image::convexHull() {
    for (int ind = 0; ind < (int) contours.size(); ++ind) {
        std::vector < std::vector <Point> > sortedContours = contours;
        sort(sortedContours.begin(), sortedContours.end());
    }
}
