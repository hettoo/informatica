#include "gui.hpp"

using namespace std;

void Gui::init()
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    drawing_area = gtk_drawing_area_new();

    set_state();
    GError* error = NULL;
    gtk_window_set_icon(GTK_WINDOW(window),
            gdk_pixbuf_new_from_file(imagefile("icon.png").c_str(), &error));

    g_signal_connect(window, "destroy", G_CALLBACK(quit), NULL);
    g_signal_connect(drawing_area, "configure_event",
            G_CALLBACK(gui_configure_event), this);
    g_signal_connect(drawing_area, "expose_event",
            G_CALLBACK(gui_expose_event), this);
    gtk_widget_set_events(drawing_area, GDK_EXPOSURE_MASK
            | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);

    gtk_container_add(GTK_CONTAINER(window), drawing_area);
}

void Gui::set_state(string state)
{
    string title = PROGRAM_NAME;
    if (state != "")
    {
        title.append(" -- ");
        title.append(state);
    }
    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
}

void Gui::add_callback(const char* event, void (* callback)())
{
    g_signal_connect(window, event, callback, NULL);
}

void Gui::show()
{
    gtk_widget_show_all(window);
}

void Gui::set_ball(int index, bool value)
{
    world->set_ball(index, value);
    draw_field(index);
}

void Gui::set_ball(int x, int y, bool value)
{
    world->set_ball(x, y, value);
    draw_field(x, y);
}

void Gui::set_objects(World* new_world, Images* new_images)
{
    world = new_world;
    images = new_images;
}

void Gui::set_default_window_size(int width, int height)
{
    gtk_window_set_default_size(GTK_WINDOW(window), width, height);
}

void Gui::set_virtual_size(int width, int height)
{
    cols = width;
    rows = height;
}

void Gui::set_min_field_pixels(int pixels)
{
    min_field_pixels = pixels;
}

void Gui::set_max_field_pixels(int pixels)
{
    max_field_pixels = pixels;
}

void Gui::set_scroll_margin(float margin)
{
    scroll_margin = margin;
}

void Gui::set_field_size(int size)
{
    field_size = size;
    images->scale(field_size, field_size);
}

bool Gui::get_ignore_errors()
{
    return ignore_errors;
}

void Gui::set_ignore_errors(bool value)
{
    ignore_errors = value;
}

void Gui::toggle_fullscreen()
{
    fullscreen = !fullscreen;
    apply_window_state();
}

void Gui::set_fullscreen(bool value)
{
    fullscreen = value;
    apply_window_state();
}

void Gui::apply_window_state()
{
    if (fullscreen)
        gtk_window_fullscreen(GTK_WINDOW(window));
    else
        gtk_window_unfullscreen(GTK_WINDOW(window));
}

void Gui::zoom(int index)
{
    int new_field_size = field_size + index;
    if (new_field_size <= max_field_pixels
            && new_field_size >= min_field_pixels)
    {
        set_field_size(new_field_size);
        draw_all();
        redraw();
    }
}

void Gui::move_camera_x(int x)
{
    view_x += x;
    check_view();
}

void Gui::move_camera_y(int y)
{
    view_y += y;
    check_view();
}

void Gui::align_window_x()
{
    window_width -= right_slack();
    gtk_window_resize(GTK_WINDOW(window), window_width, window_height);
}

void Gui::align_window_y()
{
    window_height -= bottom_slack();
    gtk_window_resize(GTK_WINDOW(window), window_width, window_height);
}

void Gui::align_window()
{
    align_window_x();
    align_window_y();
}

void Gui::ask_int(string msg, int& n)
{
    cout << msg << ": " << endl;
    cin >> n;
}

void Gui::ask_string(string msg, string& s)
{
    cout << msg << ": " << endl;
    cin >> s;
}

void Gui::set_surrounding_walls()
{
    for (int x = 0; x < cols; x++)
    {
        world->set_wall(x, 0, true);
        world->set_wall(x, rows - 1, true);
    }
    for (int y = 1; y < rows - 1; y++)
    {
        world->set_wall(0, y, true);
        world->set_wall(cols - 1, y, true);
    }
}

void Gui::reset()
{
    world->reset();
    set_surrounding_walls();
}

void Gui::queue_draw()
{
    gtk_widget_queue_draw(drawing_area);
}

void Gui::redraw()
{
    check_world();
    queue_draw();
}

void Gui::draw_rect(int x, int y, int width, int height, int red, int green,
        int blue)
{
    cairo_t* cr;

    cr = gdk_cairo_create(pixmap);

    GdkRectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;

    GdkColor color;
    color.red = red;
    color.green = green;
    color.blue = blue;

    gdk_cairo_set_source_color(cr, &color);
    gdk_cairo_rectangle(cr, &rect);

    cairo_fill(cr);

    cairo_destroy(cr);
}

void Gui::draw_img(cairo_surface_t* img, int x, int y)
{
    cairo_t* cr = gdk_cairo_create(pixmap);
    cairo_set_source_surface(cr, img, x, y);
    cairo_paint(cr);
    cairo_destroy(cr);
}

void Gui::draw_ground(int index)
{
    draw_img(images->ground, world->get_x(index) * field_size,
            world->get_y(index) * field_size);
}

void Gui::draw_wall(int index)
{
    draw_img(images->wall_base, world->get_x(index) * field_size,
            world->get_y(index) * field_size);

    for (int i = 0; i < DIRECTIONS; i++)
    {
        int x = world->get_x(index) + dir_x(i);
        int y = world->get_y(index) + dir_y(i);
        if (x >= 0 && x < cols && y >= 0 && y < rows && world->get_wall(x, y))
            draw_img(images->wall[i], world->get_x(index) * field_size,
                    world->get_y(index) * field_size);
    }
}

void Gui::draw_ball(int index)
{
    draw_img(images->ball, world->get_x(index) * field_size, world->get_y(index)
            * field_size);
}

void Gui::draw_robot(int index)
{
    draw_img(world->robot_invincible() ? images->super[world->robot_dir()]
            : images->robot[world->robot_dir()], world->get_x(index)
            * field_size, world->get_y(index) * field_size);
}

void Gui::draw_field(int index)
{
    draw_ground(index);
    if (world->get_wall(index))
        draw_wall(index);
    if (world->get_ball(index))
        draw_ball(index);
    if (world->get_robot(index))
        draw_robot(index);
}

void Gui::draw_field(int x, int y)
{
    draw_field(world->item(x, y));
}

void Gui::draw_neighbours(int x, int y)
{
    for (int i = 0; i < DIRECTIONS; i++)
    {
        int new_x = x + dir_x(i);
        int new_y = y + dir_y(i);
        if (new_x >= 0 && new_x < cols && new_y >= 0 && new_y < rows)
            draw_field(new_x, new_y);
    }
}

void Gui::draw_neighbours(int index)
{
    draw_neighbours(world->get_x(index), world->get_y(index));
}

void Gui::draw_all()
{
    draw_rect(0, 0, max_field_pixels * cols, max_field_pixels * rows, 0, 0, 0);
    for (int i = 0; i < cols * rows; i++)
        draw_field(i);
}

int Gui::virtual_x(int x)
{
    return (x + view_x) / field_size;
}

int Gui::virtual_y(int y)
{
    return (y + view_y) / field_size;
}

int Gui::right_slack()
{
    return window_width - cols * field_size;
}

int Gui::bottom_slack()
{
    return window_height - rows * field_size;
}

int Gui::right_slack_view()
{
    return window_width + view_x - cols * field_size;
}

int Gui::bottom_slack_view()
{
    return window_height + view_y - rows * field_size;
}

void Gui::check_view()
{
    if (right_slack_view() > 0)
        view_x -= right_slack_view();
    if (view_x < 0 || right_slack() >= 0)
        view_x = 0;
    if (bottom_slack_view() > 0)
        view_y -= bottom_slack_view();
    if (view_y < 0 || bottom_slack() >= 0)
        view_y = 0;
}

void Gui::check_world()
{
    int robot_x = world->robot_x();
    int robot_y = world->robot_y();

    if ((robot_x + 1) * field_size > view_x + window_width / (1 + scroll_margin))
    {
        view_x = robot_x * field_size - window_width / 2;
        if (view_x + window_width > cols * field_size)
            view_x = cols * field_size - window_width;
    }
    else if (robot_x * field_size < view_x + window_width * scroll_margin)
    {
        view_x = robot_x * field_size - window_width / 4;
    }
    if ((robot_y + 1) * field_size > view_y + window_height / (1
                + scroll_margin))
    {
        view_y = robot_y * field_size - window_height / 2;
        if(view_y + window_height > rows * field_size)
            view_y = rows * field_size - window_height;
    }
    else if (robot_y * field_size < view_y + window_height * scroll_margin)
    {
        view_y = robot_y * field_size - window_width / 4;
    }

    check_view();
}

bool Gui::configure()
{
    window_width = drawing_area->allocation.width;
    window_height = drawing_area->allocation.height;

    window_rect.x = 0;
    window_rect.y = 0;
    window_rect.width = window_width;
    window_rect.height = window_height;

    if (!pixmap)
    {
        pixmap = gdk_pixmap_new(drawing_area->window, max_field_pixels * cols,
                max_field_pixels * rows, -1);
        draw_all();
    }
    redraw();

    return true;
}

bool Gui::expose()
{
    cairo_t* cr = gdk_cairo_create(drawing_area->window);
    gdk_cairo_set_source_pixmap(cr, pixmap, -view_x, -view_y);
    gdk_cairo_rectangle(cr, &window_rect);
    cairo_fill(cr);
    cairo_destroy(cr);

    return true;
}

void Gui::error(const string msg)
{
    if (!ignore_errors)
        ::error(msg);
}

bool gui_configure_event(GtkWidget* widget, GdkEventConfigure* event, Gui* self)
{
    return self->configure();
}

bool gui_expose_event(GtkWidget* widget, GdkEventExpose* event, Gui* self)
{
    return self->expose();
}

