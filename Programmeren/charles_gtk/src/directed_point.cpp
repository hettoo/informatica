#include "directed_point.hpp"

int DirectedPoint::get_x()
{
    return x;
}

int DirectedPoint::get_y()
{
    return y;
}

int DirectedPoint::get_dir()
{
    return dir;
}

void DirectedPoint::set(int new_x, int new_y, int new_dir)
{
    x = new_x;
    y = new_y;
    dir = new_dir;
}

void DirectedPoint::set_x(int new_x)
{
    x = new_x;
}

void DirectedPoint::set_y(int new_y)
{
    y = new_y;
}

void DirectedPoint::set_dir(int new_dir)
{
    dir = new_dir;
}

void DirectedPoint::step()
{
    switch (dir)
    {
        case DIR_NORTH:
            y--;
            break;
        case DIR_EAST:
            x++;
            break;
        case DIR_SOUTH:
            y++;
            break;
        case DIR_WEST:
            x--;
            break;
    }
}

void DirectedPoint::turn_left()
{
    if (dir == 0)
        dir = DIRECTIONS - 1;
    else
        dir--;
}

void DirectedPoint::turn_right()
{
    dir = (dir + 1) % DIRECTIONS;
}

bool DirectedPoint::north()
{
    return dir == DIR_NORTH;
}
