#include "globals.hpp"

using namespace std;

int dir_x(const int dir)
{
    return (1 - 2 * (dir / 2)) * dir % 2;
}

int dir_y(const int dir)
{
    return (2 * (dir / 2) - 1) * (dir + 1) % 2;
}

void print(const string msg)
{
    cout << msg;
}

void say(const string msg)
{
    print(msg);
    cout << endl;
}

void error(const string msg)
{
    string str;
    str.append("ERROR: ");
    str.append(msg);
    say(str);
    quit();
}

string datafile(const string file)
{
    string str;
    str.append(DATA_DIR);
    str.append(file);
    return str;
}

string imagefile(const string file)
{
    string str;
    str.append(DATA_DIR);
    str.append(IMAGE_DATA_DIR);
    str.append(file);
    return str;
}

string labyrinthfile(const string file)
{
    string str;
    str.append(DATA_DIR);
    str.append(LABYRINTH_DATA_DIR);
    str.append(file);
    return str;
}

void quit()
{
    exit(0);
}
