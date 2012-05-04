#ifndef __CHARLES_IMAGES_H__
#define __CHARLES_IMAGES_H__

#include <sstream>
#include <cairo.h>

#include "globals.hpp"

class Images
{
    public:
        cairo_surface_t* ground_big;
        cairo_surface_t* ball_big;
        cairo_surface_t* wall_base_big;
        cairo_surface_t* wall_big[DIRECTIONS];
        cairo_surface_t* robot_big[DIRECTIONS];
        cairo_surface_t* super_big[DIRECTIONS];

        cairo_surface_t* ground;
        cairo_surface_t* ball;
        cairo_surface_t* wall_base;
        cairo_surface_t* wall[DIRECTIONS];
        cairo_surface_t* robot[DIRECTIONS];
        cairo_surface_t* super[DIRECTIONS];

        Images();

        void scale(int width, int height);
};

cairo_surface_t* scale_surface(cairo_surface_t* src, int width, int height);

#endif
