#include "images.hpp"

using namespace std;

Images::Images()
{
    ground_big = cairo_image_surface_create_from_png(imagefile(
                "ground.png").c_str());
    ball_big = cairo_image_surface_create_from_png(imagefile(
                "ball.png").c_str());
    wall_base_big = cairo_image_surface_create_from_png(imagefile(
                "wall.png").c_str());

    for (int i = 0; i < DIRECTIONS; i++)
    {
        stringstream iterator;
        iterator << i;

        string str = "wall_";
        str.append(iterator.str());
        str.append(".png");
        wall_big[i] = cairo_image_surface_create_from_png(imagefile(
                    str).c_str());

        str = "robot_";
        str.append(iterator.str());
        str.append(".png");
        robot_big[i] = cairo_image_surface_create_from_png(imagefile(
                    str).c_str());

        str = "super_";
        str.append(iterator.str());
        str.append(".png");
        super_big[i] = cairo_image_surface_create_from_png(imagefile(
                    str).c_str());
    }
}

void Images::scale(int width, int height)
{
    ground = scale_surface(ground_big, width, height);
    ball = scale_surface(ball_big, width, height);
    wall_base = scale_surface(wall_base_big, width, height);
    for (int i = 0; i < DIRECTIONS; i++)
    {
        wall[i] = scale_surface(wall_big[i], width, height);
        robot[i] = scale_surface(robot_big[i], width, height);
        super[i] = scale_surface(super_big[i], width, height);
    }
}

cairo_surface_t* scale_surface(cairo_surface_t* src, int width, int height)
{
    cairo_surface_t* dest = cairo_surface_create_similar(src,
            CAIRO_CONTENT_COLOR_ALPHA, width, height);
    cairo_t* cr = cairo_create(dest);
    cairo_scale(cr, (double)width / cairo_image_surface_get_width(src),
            (double)height / cairo_image_surface_get_height(src));
    cairo_set_source_surface(cr, src, 0, 0);
    cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REFLECT);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
    return dest;
}
