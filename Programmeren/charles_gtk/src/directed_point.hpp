#ifndef __CHARLES_DIRECTED_POINT_H__
#define __CHARLES_DIRECTED_POINT_H__

#include "globals.hpp"

class DirectedPoint
{
    int x;
    int y;
    int dir;

    public:

    int get_x();
    int get_y();
    int get_dir();

    void set(int new_x, int new_y, int new_dir);
    void set_x(int new_x);
    void set_y(int new_y);
    void set_dir(int new_dir);

    void step();
    void turn_left();
    void turn_right();
    bool north();
};

#endif
