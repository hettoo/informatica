#ifndef __CHARLES_ROBOT_H__
#define __CHARLES_ROBOT_H__

#include "directed_point.hpp"

class Robot
{
    DirectedPoint position;
    DirectedPoint default_position;
    bool invincible;

    int max_x;
    int max_y;

    public:

    Robot();

    int get_x();
    int get_y();
    int get_dir();
    bool is_invincible();

    int next_x();
    int next_y();

    void set(int new_x, int new_y, int new_dir);
    void set_default(int new_x, int new_y, int new_dir);
    void set_max(int new_max_x, int new_max_y);
    void toggle_invincible();

    void reset();

    void step();
    void turn_left();
    void turn_right();
    bool north();
};

#endif
