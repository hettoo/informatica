#ifndef __CHARLES_GUI_H__
#define __CHARLES_GUI_H__

#include <gtk/gtk.h>

#include "globals.hpp"
#include "world.hpp"
#include "images.hpp"

class Gui
{
    GtkWidget* window;
    GtkWidget* drawing_area;
    GdkPixmap* pixmap;

    World* world;
    Images* images;

    int window_width;
    int window_height;
    GdkRectangle window_rect;

    int cols;
    int rows;

    float scroll_margin;
    int min_field_pixels;
    int max_field_pixels;
    int field_size;

    int view_x;
    int view_y;
    bool fullscreen;

    bool ignore_errors;

    public:

    void init();
    void set_state(std::string state = "");

    void add_callback(const char* event, void (* callback)());

    void set_objects(World* new_world, Images* new_images);
    void set_default_window_size(int width, int height);
    void set_virtual_size(int width, int height);
    void set_min_field_pixels(int pixels);
    void set_max_field_pixels(int pixels);
    void set_scroll_margin(float margin);
    void set_field_size(int size);
    bool get_ignore_errors();
    void set_ignore_errors(bool value);
    void set_fullscreen(bool value);
    void toggle_fullscreen();

    void reset();
    void queue_draw();
    void redraw();
    void show();

    void zoom(int index);
    void move_camera_x(int x);
    void move_camera_y(int y);
    void align_window();

    void ask_int(std::string msg, int& n);
    void ask_string(std::string msg, std::string& s);

    void set_surrounding_walls();
    void set_ball(int index, bool value);
    void set_ball(int x, int y, bool value);

    void draw_ground(int index);
    void draw_wall(int index);
    void draw_ball(int index);
    void draw_robot(int index);
    void draw_field(int index);
    void draw_field(int x, int y);
    void draw_neighbours(int x, int y);
    void draw_neighbours(int index);
    void draw_all();

    int virtual_x(int x);
    int virtual_y(int y);

    void error(const std::string msg);

    bool configure();
    bool expose();

    private:

    void draw_rect(int x, int y, int width, int height, int red, int green,
            int blue);
    void draw_img(cairo_surface_t* img, int x, int y);

    int right_slack();
    int bottom_slack();
    int right_slack_view();
    int bottom_slack_view();

    void align_window_x();
    void align_window_y();

    void apply_window_state();
    void check_view();
    void check_world();
};

bool gui_configure_event(GtkWidget* widget, GdkEventConfigure* event,
        Gui* self);
bool gui_expose_event(GtkWidget* widget, GdkEventExpose* event, Gui* self);

#endif
