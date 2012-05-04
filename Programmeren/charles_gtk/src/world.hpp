#ifndef __CHARLES_WORLD_H__
#define __CHARLES_WORLD_H__

#include <vector>
#include <string>
#include <fstream>

#include "robot.hpp"

struct WallList
{
    int x;
    int y;
    int size;

    WallList():
        size(0)
    {}
};

struct WallGroup
{
    int x;
    int y;
    int width;
    int height;

    WallGroup():
        width(0),
        height(0)
    {}
};

struct Node
{
    int x;
    int y;
};

struct AStarNode
{
    std::vector<int> path;
    int cost;
    int dist;
};

const int UNKNOWN = -1;

class World
{
    int cols;
    int rows;

    Robot* robot;
    std::vector<bool> walls;
    std::vector<bool> balls;

    public:

    World(int cols, int rows);

    void set_robot(Robot* new_robot);

    void read_file(const std::string filename);
    void write_file(const std::string filename);
    void reset();

    int item(int x, int y);
    int get_x(int index);
    int get_y(int index);
    int get_direction(int x, int y, int index);

    bool get_robot(int index);
    bool get_robot(int x, int y);
    bool get_ball(int index);
    bool get_ball(int x, int y);
    bool get_wall(int index);
    bool get_wall(int x, int y);
    void set_ball(int index, bool value);
    void set_ball(int x, int y, bool value);
    void set_wall(int index, bool value);
    void set_wall(int x, int y, bool value);

    int robot_x();
    int robot_y();
    int robot_dir();
    bool robot_invincible();

    void generate_maze();

    void find_path(int start_x, int start_y, int goal_x, int goal_y,
            std::vector<int>& path);

    private:

    void find_ball(int& x, int& y);
    void get_wall_groups(std::vector<WallGroup>& wall_groups);
    void disassemble_wall_group_x(WallGroup& wall_group,
            std::vector<WallList>& walls_x);
    void disassemble_wall_group_y(WallGroup& wall_group,
            std::vector<WallList>& walls_y);
    void disassemble_wall_group(WallGroup& wall_group,
            std::vector<WallList>& walls_x, std::vector<WallList>& walls_y);
    void disassemble_wall_groups(std::vector<WallGroup>& wall_groups,
        std::vector<WallList>& walls_x, std::vector<WallList>& walls_y);
    int manhattan(Node& a, Node& b);
    void add_a_star_node(std::vector<AStarNode>& frontier,
            std::vector<Node>& nodes, int position, int goal,
            int previous = UNKNOWN);
    bool node_reachable(Node& a, Node& b);
    void add_a_star_connected_nodes(std::vector<AStarNode>& frontier,
            std::vector<Node>& nodes, int current);
    bool is_node(int x, int y);
    void find_nodes(std::vector<Node>& nodes, int start_x, int start_y,
            int goal_x, int goal_y);
    void add_path_nodes(Node& a, Node& b, std::vector<int>& path);
    void convert_path_nodes(std::vector<Node>& nodes, std::vector<int>& indices,
            std::vector<int>& path);
};

#endif
