#ifndef __CHARLES_USER_H__
#define __CHARLES_USER_H__

#include <string>

#include "src/globals.hpp"

/* Customisable constants */
const int FIELD_COLS = 50;
const int FIELD_ROWS = 32;
const std::string DEFAULT_LABYRINTH = "labyrinth";
const int  DEFAULT_WINDOW_WIDTH  = 900;
const int  DEFAULT_WINDOW_HEIGHT = 600;
const bool START_FULLSCREEN      = false;
const float SCROLL_MARGIN = 0.08;
const int DELAY_FAST    = 12;
const int DELAY_NORMAL  = 30;
const int DELAY_SLOW    = 160;
const int DELAY_DEFAULT = DELAY_FAST;
const int START_X   = 1;
const int START_Y   = 15;
const int START_DIR = DIR_EAST;
const int CAMERA_STEP = 50;
const int ZOOM_STEP            = 4;
const int MIN_FIELD_PIXELS     = 12;
const int MAX_FIELD_PIXELS     = 128;
const int DEFAULT_FIELD_PIXELS = 32;

/* Usable procedures */
void step();
void steps(int n);
void turn_left();
void turn_right();
void get_ball();
void put_ball();

/* Usable functions */
bool in_front_of_wall();
bool on_ball();
bool north();

/* Other procedures required in the custom part */
void draw_line_with_balls(int n);
void make_string_with_balls();
void make_chaos_with_balls();
void make_path_with_balls();
void make_labyrinth();

/* User functions */
void start();
void clean_string_with_balls();
void clean_chaos_with_balls();
void follow_path();
void leave_labyrinth();
void write_number(int positive_decimal);
void add();
void subtract();
void multiply();

#endif
