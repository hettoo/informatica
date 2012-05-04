#ifndef __CHARLES_GLOBALS_H__
#define __CHARLES_GLOBALS_H__

#include <cstdlib>
#include <iostream>
#include <string>

const std::string PROGRAM_NAME = "Charles GTK";
const std::string DATA_DIR = "data/";
const std::string IMAGE_DATA_DIR = "images/";
const std::string LABYRINTH_DATA_DIR = "labyrinths/";

enum Direction
{
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST,
    DIRECTIONS
};

int dir_x(const int dir);
int dir_y(const int dir);
void print(const std::string msg);
void say(const std::string msg);
void error(const std::string msg);
std::string datafile(const std::string file);
std::string imagefile(const std::string file);
std::string labyrinthfile(const std::string file);
void quit();

#endif
