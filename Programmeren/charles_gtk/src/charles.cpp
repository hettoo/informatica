#include "../user.hpp"
#include "charles.hpp"

using namespace std;

World world(FIELD_COLS, FIELD_ROWS);
Robot charles;
Images images;
Gui gui;

bool sticky_ball = false;
bool sticky_wall = false;

void* (*func)(void*) = NULL;
volatile bool updated = false;

int event_x, event_y;
vector<int> path;

int speed = DELAY_DEFAULT * DELAY_MULTIPLIER;

void do_delay()
{
    gdk_threads_enter();
    gui.redraw();
    gdk_threads_leave();
    usleep(speed);
}

void update_title()
{
    stringstream state;
    state << "(" << charles.get_x() << "," << charles.get_y() << ")";
    gui.set_state(state.str());
}

void step()
{
    int old_x = charles.get_x();
    int old_y = charles.get_y();
    charles.step();
    int index = world.item(charles.get_x(), charles.get_y());
    if(world.get_wall(index))
        gui.error("Charles died in a terrible accident involving a wall :-(");
    if (sticky_ball)
        world.set_ball(index, !world.get_ball(index));
    gdk_threads_enter();
    update_title();
    if (sticky_wall)
    {
        world.set_wall(index, !world.get_wall(index));
        gui.draw_neighbours(index);
    }
    gui.draw_field(old_x, old_y);
    gui.draw_field(index);
    gdk_threads_leave();
    do_delay();
}

void steps(int n)
{
    for (int i = 0; i < n; i++)
        step();
}

void turn_left()
{
    charles.turn_left();
    gdk_threads_enter();
    gui.draw_field(charles.get_x(), charles.get_y());
    gdk_threads_leave();
    do_delay();
}

void turn_right()
{
    charles.turn_right();
    gdk_threads_enter();
    gui.draw_field(charles.get_x(), charles.get_y());
    gdk_threads_leave();
    do_delay();
}

void get_ball()
{
    int index = world.item(charles.get_x(), charles.get_y());
    if (!world.get_ball(index))
        gui.error("Ball not set");
    gdk_threads_enter();
    gui.set_ball(index, false);
    gdk_threads_leave();
    do_delay();
}

void put_ball()
{
    int index = world.item(charles.get_x(), charles.get_y());
    if (world.get_ball(index))
        gui.error("Ball already set");
    gdk_threads_enter();
    gui.set_ball(index, true);
    gdk_threads_leave();
    do_delay();
}

void draw_line_with_balls( int n)
{
    for (int i = 1; i < n; i++)
    {
        put_ball();
        step();
    }
}

bool north()
{
    return charles.north();
}

bool in_front_of_wall()
{
    return world.get_wall(charles.next_x(), charles.next_y());
}

bool on_ball()
{
    return world.get_ball(charles.get_x(), charles.get_y());
}

void make_string_with_balls()
{
    gdk_threads_enter();
    for (int x = 1; x < FIELD_COLS - 1; x++)
    {
        gui.set_ball(x, 1, true);
        gui.set_ball(x, FIELD_ROWS - 2, true);
    }
    for (int y = 2; y < FIELD_ROWS - 2; y++)
    {
        gui.set_ball(1, y, true);
        gui.set_ball(FIELD_COLS - 2, y, true);
    }
    gdk_threads_leave();
    do_delay();
}

void make_chaos_with_balls()
{
    gdk_threads_enter();
    int start_row = 1 + rand() % 3;
    int end_row = 5 + rand() % 14;
    for (int y = start_row; y <= end_row; y++)
    {
        int row_length = rand() % 20;
        for (int x = FIELD_COLS - 2; x > FIELD_COLS - row_length - 2; x--)
            gui.set_ball(x, y, true);
    }
    gdk_threads_leave();
    do_delay();
}

void make_path_with_balls()
{
    gdk_threads_enter();
    for (int x = 1; x < FIELD_COLS - 4; x++)
        gui.set_ball(x, START_Y, true);
    for (int y = START_Y; y < FIELD_ROWS - 4; y++)
        gui.set_ball(FIELD_COLS - 5, y, true);
    for (int x = FIELD_COLS - 5; x >= FIELD_COLS / 2; x--)
        gui.set_ball(x, FIELD_ROWS - 5, true);
    for (int y = FIELD_ROWS - 5; y >= 6; y--)
        gui.set_ball(FIELD_COLS / 2, y, true);
    for (int x = FIELD_COLS / 2; x >= 7; x--)
        gui.set_ball(x, 6, true);
    for (int y = 6; y <= FIELD_ROWS / 2 - 3; y++)
        gui.set_ball(7, y, true);
    gdk_threads_leave();
    do_delay();
}

void redraw()
{
    update_title();
    gui.draw_all();
    gui.redraw();
}

void make_labyrinth()
{
    world.read_file(labyrinthfile(DEFAULT_LABYRINTH));
    gdk_threads_enter();
    redraw();
    gdk_threads_leave();
    do_delay();
}

void set_given_numbers()
{
    int n, m;
    gui.ask_int("Number1", n);
    gui.ask_int("Number2", m);

    gui.set_ignore_errors(true);
    while (!north())
        turn_left();
    while (!in_front_of_wall())
        step();
    turn_right();
    if (n == 0)
    {
        turn_right();
        step();
    }
    else
    {
        steps(FIELD_COLS - 3 - floor(log(n) / log(2)));
        write_number(n);
        turn_left();
        turn_left();
        step();
        while (!in_front_of_wall())
            step();
        turn_left();
        step();
    }
    turn_left();
    if (m == 0)
    {
        turn_right();
    }
    else
    {
        steps(FIELD_COLS - 3 - floor(log(m) / log(2)));
        write_number(m);
        turn_left();
        turn_left();
        step();
        while (!in_front_of_wall())
            step();
        turn_left();
    }
    steps(START_Y - 2);
    turn_left();
    steps(START_X - 1);
    gui.set_ignore_errors(false);
}

void* call_turn_left(void* t)
{
    turn_left();
    return t;
}

void* call_turn_right(void* t)
{
    turn_right();
    return t;
}

void* call_step(void* t)
{
    step();
    return t;
}

void* call_toggle_ball(void* t)
{
    if (world.get_ball(charles.get_x(), charles.get_y()))
        get_ball();
    else
        put_ball();
    return t;
}

void* call_start(void* t)
{
    start();
    return t;
}

void* call_clean_string_with_balls(void* t)
{
    clean_string_with_balls();
    return t;
}

void* call_clean_chaos_with_balls(void* t)
{
    clean_chaos_with_balls();
    return t;
}

void* call_follow_path(void* t)
{
    follow_path();
    return t;
}

void* call_leave_labyrinth(void* t)
{
    leave_labyrinth();
    return t;
}

void* call_write_number(void* t)
{
    int n;
    gui.ask_int("Number", n);
    write_number(n);
    return t;
}

void* call_add(void* t)
{
    set_given_numbers();
    add();
    return t;
}

void* call_subtract(void* t)
{
    set_given_numbers();
    subtract();
    return t;
}

void* call_multiply(void* t)
{
    set_given_numbers();
    multiply();
    return t;
}

int ncmod(int n, int mod)
{
    if (n < 0)
        return mod - -n % mod;
    return n % mod;
}

void turn_to(int src_dir, int dest_dir)
{
    int turns_left = ncmod(src_dir - dest_dir, DIRECTIONS);
    int turns_right = ncmod(dest_dir - src_dir, DIRECTIONS);
    if (turns_left < turns_right)
    {
        while (turns_left-- > 0)
            turn_left();
    }
    else
    {
        while (turns_right-- > 0)
            turn_right();
    }
}

void* call_walk_path(void* t)
{
    path.resize(0);
    world.find_path(charles.get_x(), charles.get_y(),
            gui.virtual_x(event_x), gui.virtual_y(event_y),
            path);
    for (size_t i = 0; i < path.size(); i++)
    {
        int direction = world.get_direction(charles.get_x(), charles.get_y(),
                path[i]);
        turn_to(charles.get_dir(), direction);
        step();
    }
    return t;
}

void* executer(void* t)
{
    while (1)
    {
        if (updated)
        {
            func(t);
            updated = false;
        }
    }
    return t;
}

void thread_func(void* (function)(void*))
{
    func = function;
    updated = true;
}

void reset()
{
    gui.reset();
    redraw();
}

void toggle_ball()
{
    int x = charles.get_x();
    int y = charles.get_y();
    world.set_ball(x, y, !world.get_ball(x, y));
    gui.draw_field(x, y);
    gui.redraw();
}

void toggle_wall()
{
    int x = charles.get_x();
    int y = charles.get_y();
    world.set_wall(x, y, !world.get_wall(x, y));
    gui.draw_field(x, y);
    gui.draw_neighbours(x, y);
    gui.redraw();
}

void toggle_sticky_ball()
{
    sticky_ball = !sticky_ball;
    if (sticky_ball)
        toggle_ball();
}

void toggle_sticky_wall()
{
    sticky_wall = !sticky_wall;
    if (sticky_wall)
        toggle_wall();
}

void toggle_invincibility()
{
    gui.set_ignore_errors(!gui.get_ignore_errors());
    charles.toggle_invincible();
    gui.draw_field(charles.get_x(), charles.get_y());
    gui.redraw();
}

void open_custom_file()
{
    string filename;
    gui.ask_string("Filename", filename);
    world.read_file(labyrinthfile(filename));
    redraw();
}

void save_custom_file()
{
    string filename;
    gui.ask_string("Filename", filename);
    world.write_file(labyrinthfile(filename));
}

void move_camera_x(int x)
{
    gui.move_camera_x(x);
    gui.queue_draw();
}

void move_camera_y(int y)
{
    gui.move_camera_y(y);
    gui.queue_draw();
}

void generate_maze()
{
    world.generate_maze();
    gui.draw_all();
    gui.redraw();
}

bool key_press(GtkWidget* widget, GdkEventKey* event)
{
    switch (event->keyval)
    {
        case GDK_KEY_q:
            quit();
            break;
        case GDK_KEY_f:
            gui.toggle_fullscreen();
            break;
        case GDK_KEY_comma:
            gui.zoom(ZOOM_STEP);
            break;
        case GDK_KEY_period:
            gui.zoom(-ZOOM_STEP);
            break;
        case GDK_KEY_Left:
            move_camera_x(-CAMERA_STEP);
            break;
        case GDK_KEY_Right:
            move_camera_x(CAMERA_STEP);
            break;
        case GDK_KEY_Up:
            move_camera_y(-CAMERA_STEP);
            break;
        case GDK_KEY_Down:
            move_camera_y(CAMERA_STEP);
            break;
        case GDK_KEY_a:
            gui.align_window();
            break;
        case GDK_KEY_plus:
            speed = DELAY_FAST * DELAY_MULTIPLIER;
            break;
        case GDK_KEY_equal:
            speed = DELAY_NORMAL * DELAY_MULTIPLIER;
            break;
        case GDK_KEY_minus:
            speed = DELAY_SLOW * DELAY_MULTIPLIER;
            break;
        case GDK_KEY_r:
            reset();
            break;
        case GDK_KEY_j:
            thread_func(call_turn_left);
            break;
        case GDK_KEY_l:
            thread_func(call_turn_right);
            break;
        case GDK_KEY_k:
            thread_func(call_step);
            break;
        case GDK_KEY_b:
            thread_func(call_toggle_ball);
            break;
        case GDK_KEY_w:
            toggle_wall();
            break;
        case GDK_KEY_B:
            toggle_sticky_ball();
            break;
        case GDK_KEY_W:
            toggle_sticky_wall();
            break;
        case GDK_KEY_i:
            toggle_invincibility();
            break;
        case GDK_KEY_o:
            open_custom_file();
            break;
        case GDK_KEY_s:
            save_custom_file();
            break;
        case GDK_KEY_m:
            generate_maze();
            break;
        case GDK_KEY_1:
            thread_func(call_start);
            break;
        case GDK_KEY_2:
            thread_func(call_clean_string_with_balls);
            break;
        case GDK_KEY_3:
            thread_func(call_clean_chaos_with_balls);
            break;
        case GDK_KEY_4:
            thread_func(call_follow_path);
            break;
        case GDK_KEY_5:
            thread_func(call_leave_labyrinth);
            break;
        case GDK_KEY_6:
            thread_func(call_write_number);
            break;
        case GDK_KEY_7:
            thread_func(call_add);
            break;
        case GDK_KEY_8:
            thread_func(call_subtract);
            break;
        case GDK_KEY_9:
            thread_func(call_multiply);
            break;
    }
    return true;
}

bool button_press(GtkWidget* widget, GdkEventButton* event)
{
    event_x = event->x;
    event_y = event->y;
    if (event->button == 3)
        thread_func(call_walk_path);
    return true;
}

int main(int argc, char* argv[])
{
    g_thread_init(NULL);
    gdk_threads_init();

    gtk_init(&argc, &argv);

    charles.set_default(START_X, START_Y, START_DIR);

    gui.init();

    world.set_robot(&charles);
    gui.set_objects(&world, &images);
    gui.set_default_window_size(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    gui.set_virtual_size(FIELD_COLS, FIELD_ROWS);
    gui.set_min_field_pixels(MIN_FIELD_PIXELS);
    gui.set_max_field_pixels(MAX_FIELD_PIXELS);
    gui.set_scroll_margin(SCROLL_MARGIN);

    gui.add_callback("key_press_event", G_CALLBACK(key_press));
    gui.add_callback("button_press_event", G_CALLBACK(button_press));

    gui.set_field_size(DEFAULT_FIELD_PIXELS);
    gui.set_fullscreen(START_FULLSCREEN);
    gui.reset();
    gui.show();
    update_title();

    pthread_t thread;
    pthread_create(&thread, NULL, executer, NULL);

    gtk_main();

    return 0;
}
