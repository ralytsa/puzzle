#ifndef POINT_H
#define POINT_H

class Point {
public:
    Point() : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}
    ~Point() {}
    bool operator <(const Point& p) const {
        return x < p.x || (x == p.x && y < p.y);
    }

    int x, y;
};

#endif
