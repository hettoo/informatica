#include "robot.hpp"

Robot::Robot()
{
    invincible = false;
}

int Robot::get_x()
{
    return position.get_x();
}

int Robot::get_y()
{
    return position.get_y();
}

int Robot::get_dir()
{
    return position.get_dir();
}

bool Robot::is_invincible()
{
    return invincible;
}

int Robot::next_x()
{
    DirectedPoint next_position = position;
    next_position.step();
    return next_position.get_x();
}

int Robot::next_y()
{
    DirectedPoint next_position = position;
    next_position.step();
    return next_position.get_y();
}

void Robot::set(int new_x, int new_y, int new_dir)
{
    position.set(new_x, new_y, new_dir);
}

void Robot::set_default(int new_x, int new_y, int new_dir)
{
    default_position.set(new_x, new_y, new_dir);
    position = default_position;
}

void Robot::set_max(int new_max_x, int new_max_y)
{
    max_x = new_max_x;
    max_y = new_max_y;
}

void Robot::toggle_invincible()
{
    invincible = !invincible;
}

void Robot::reset()
{
    position = default_position;
}

void Robot::step()
{
    position.step();
    if (get_x() == -1)
        position.set_x(max_x - 1);
    else if (get_x() == max_x)
        position.set_x(0);
    if (get_y() == -1)
        position.set_y(max_y - 1);
    else if (get_y() == max_y)
        position.set_y(0);
}

void Robot::turn_left()
{
    position.turn_left();
}

void Robot::turn_right()
{
    position.turn_right();
}

bool Robot::north()
{
    return position.north();
}
