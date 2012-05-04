#include "world.hpp"

using namespace std;

World::World(int new_cols, int new_rows)
{
    cols = new_cols;
    rows = new_rows;
    walls.reserve(cols * rows);
    balls.reserve(cols * rows);
}

void World::set_robot(Robot* new_robot)
{
    robot = new_robot;
    robot->set_max(cols, rows);
}

void World::reset()
{
    for (int i = 0; i < cols * rows; i++)
    {
        balls[i] = false;
        walls[i] = false;
    }
    robot->reset();
}

int World::item(int x, int y)
{
    return y * cols + x;
}

int World::get_x(int index)
{
    return index % cols;
}

int World::get_y(int index)
{
    return int(index / cols);
}

int World::get_direction(int x, int y, int index)
{
    if (y > get_y(index))
        return DIR_NORTH;
    if (y < get_y(index))
        return DIR_SOUTH;
    if (x > get_x(index))
        return DIR_WEST;
    return DIR_EAST;
}

bool World::get_robot(int index)
{
    return robot->get_x() == get_x(index) && robot->get_y() == get_y(index);
}

bool World::get_robot(int x, int y)
{
    return robot->get_x() == x && robot->get_y() == y;
}

bool World::get_ball(int index)
{
    return balls[index];
}

bool World::get_ball(int x, int y)
{
    return balls[item(x, y)];
}

bool World::get_wall(int index)
{
    return walls[index];
}

bool World::get_wall(int x, int y)
{
    return walls[item(x, y)];
}

void World::set_ball(int index, bool value)
{
    balls[index] = value;
}

void World::set_ball(int x, int y, bool value)
{
    balls[item(x, y)] = value;
}

void World::set_wall(int index, bool value)
{
    walls[index] = value;
}

void World::set_wall(int x, int y, bool value)
{
    walls[item(x, y)] = value;
}

int World::robot_x()
{
    return robot->get_x();
}

int World::robot_y()
{
    return robot->get_y();
}

int World::robot_dir()
{
    return robot->get_dir();
}

bool World::robot_invincible()
{
    return robot->is_invincible();
}

void World::read_file(const string filename)
{
    ifstream file(filename.c_str());
    int ball_x, ball_y, walls_x, walls_y, robot_x, robot_y, robot_dir;
    file >> robot_x >> robot_y >> robot_dir >> ball_x >> ball_y >> walls_x
        >> walls_y;
    robot->set(robot_x, rows - ++robot_y, DIRECTIONS - robot_dir);
    set_ball(ball_x, rows - ++ball_y, true);
    for (int i = 0; i < walls_x; i++) {
        int x, y, d;
        file >> x >> y >> d;
        for (int j = 0; j < d; j++)
            set_wall(x + j, rows - y - 1, true);
    }
    for (int i = 0; i < walls_y; i++) {
        int x, y, d;
        file >> x >> y >> d;
        for (int j = 0; j < d; j++)
            set_wall(x, rows - (y + 1 + j), true);
    }
    file.close();
}

bool in_wall_group(WallGroup& wall_group, int x, int y)
{
    return (x >= wall_group.x && x < wall_group.x + wall_group.width)
        && (y >= wall_group.y && y < wall_group.y + wall_group.height);
}

bool in_wall_group(vector<WallGroup>& wall_groups, int x, int y)
{
    for (size_t i = 0; i < wall_groups.size(); i++)
    {
        if (in_wall_group(wall_groups[i], x, y))
            return true;
    }
    return false;
}

void World::get_wall_groups(vector<WallGroup>& wall_groups)
{
    for (int x = 1; x < cols - 1; x++)
    {
        for (int y = 1; y < rows - 1; y++)
        {
            if (get_wall(x, y) && !in_wall_group(wall_groups, x, y))
            {
                WallGroup new_group;
                new_group.x = x;
                new_group.y = y;
                new_group.width = 1;
                while (get_wall(x + new_group.width, y) && x + new_group.width
                        + 1 < cols)
                    new_group.width++;
                bool filled = true;
                while (filled && y + new_group.height + 1 < rows)
                {
                    new_group.height++;
                    for (int i = x; i < x + new_group.width; i++)
                    {
                        if (!get_wall(i, y + new_group.height))
                            filled = false;
                    }
                }
                wall_groups.push_back(new_group);
            }
        }
    }
}

void World::disassemble_wall_group_x(WallGroup& wall_group,
        vector<WallList>& walls_x)
{
    WallList wall_list;
    wall_list.x = wall_group.x;
    wall_list.y = rows - wall_group.y - 1;
    wall_list.size = wall_group.width;
    walls_x.push_back(wall_list);
}

void World::disassemble_wall_group_y(WallGroup& wall_group,
        vector<WallList>& walls_y)
{
    WallList wall_list;
    wall_list.x = wall_group.x;
    wall_list.y = rows - wall_group.y - wall_group.height;
    wall_list.size = wall_group.height;
    walls_y.push_back(wall_list);
}

void World::disassemble_wall_group(WallGroup& wall_group,
        vector<WallList>& walls_x, vector<WallList>& walls_y)
{
    if (wall_group.width > wall_group.height)
    {
        for (WallGroup group_i = wall_group; group_i.y < wall_group.y
                + wall_group.height; group_i.y++)
            disassemble_wall_group_x(group_i, walls_x);
    }
    else
    {
        for (WallGroup group_i = wall_group; group_i.x < wall_group.x
                + wall_group.width; group_i.x++)
            disassemble_wall_group_y(group_i, walls_y);
    }
}

void World::disassemble_wall_groups(vector<WallGroup>& wall_groups,
        vector<WallList>& walls_x, vector<WallList>& walls_y)
{
    for (size_t i = 0; i < wall_groups.size(); i++)
        disassemble_wall_group(wall_groups[i], walls_x, walls_y);
}

void World::find_ball(int& x, int& y)
{
    for (y = 1; y < rows - 1; y++)
    {
        for (x = 1; x < cols - 1; x++)
        {
            if (get_ball(x, y))
                return;
        }
    }
}

void World::write_file(const string filename)
{
    int ball_x, ball_y;
    find_ball(ball_x, ball_y);

    vector<WallGroup> wall_groups;
    get_wall_groups(wall_groups);

    vector<WallList> walls_x;
    vector<WallList> walls_y;

    disassemble_wall_groups(wall_groups, walls_x, walls_y);

    ofstream file(filename.c_str());
    file << robot->get_x() << " " << rows - robot->get_y() - 1 << " "
        << DIRECTIONS - robot->get_dir() << " " << ball_x << " "
        << rows - ball_y - 1 << " " << walls_x.size() << " " << walls_y.size()
        << endl;
    for (size_t i = 0; i < walls_x.size(); i++)
        file << walls_x[i].x << " " << walls_x[i].y << " " << walls_x[i].size
            << endl;
    for (size_t i = 0; i < walls_y.size(); i++)
        file << walls_y[i].x << " " << walls_y[i].y << " " << walls_y[i].size
            << endl;
    file.close();
}

int World::manhattan(Node& a, Node& b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

bool in_path(vector<int>& path, int position)
{
    for (size_t i = 0; i < path.size(); i++)
    {
        if (path[i] == position)
            return true;
    }
    return false;
}

void World::add_a_star_node(vector<AStarNode>& frontier, vector<Node>& nodes,
        int position, int goal, int previous)
{
    if (get_wall(nodes[position].x, nodes[position].y)
            || get_wall(nodes[goal].x, nodes[goal].y) || (previous != UNKNOWN
                && in_path(frontier[previous].path, position)))
        return;

    AStarNode node;
    if (previous != UNKNOWN)
        node.path = frontier[previous].path;
    node.path.push_back(position);
    node.cost = previous == UNKNOWN ? 0 : frontier[previous].cost
        + manhattan(nodes[frontier[previous].path.back()], nodes[position]);
    node.dist = manhattan(nodes[position], nodes[goal]);
    frontier.push_back(node);
}

bool World::node_reachable(Node& a, Node& b)
{
    bool x_first = true;
    for (int x = a.x; x != b.x; x += 1 - 2 * (a.x > b.x))
    {
        if (get_wall(x, a.y))
        {
            x_first = false;
            break;
        }
    }
    if (x_first)
    {
        for (int y = a.y; y != b.y; y += 1 - 2 * (a.y > b.y))
        {
            if (get_wall(b.x, y))
            {
                x_first = false;
                break;
            }
        }
    }
    if (!x_first)
    {
        for (int y = a.y; y != b.y; y += 1 - 2 * (a.y > b.y))
        {
            if (get_wall(a.x, y))
                return false;
        }
        for (int x = a.x; x != b.x; x += 1 - 2 * (a.x > b.x))
        {
            if (get_wall(x, b.y))
                return false;
        }
    }
    return true;
}

void World::add_a_star_connected_nodes(vector<AStarNode>& frontier,
        vector<Node>& nodes, int current)
{
    for (size_t i = 0; i < nodes.size(); i++)
    {
        if ((nodes[i].x != nodes[frontier[current].path.back()].x
                    || nodes[i].y != nodes[frontier[current].path.back()].y)
                && node_reachable(nodes[frontier[current].path.back()], nodes[i]))
            add_a_star_node(frontier, nodes, i, 1, current);
    }
}

size_t find_least(vector<AStarNode>& frontier)
{
    size_t least = 0;
    int least_total = UNKNOWN;
    for (size_t i = 0; i < frontier.size(); i++)
    {
        int total = frontier[i].cost + frontier[i].dist;
        if (least_total == UNKNOWN || total < least_total
                || (total == least_total
                    && frontier[i].dist < frontier[least].dist))
        {
            least = i;
            least_total = total;
        }
    }
    return least;
}

bool World::is_node(int x, int y)
{
    if (get_wall(x, y))
        return false;

    int neighbours = 0;
    for (int x_diff = -1; x_diff <= 1; x_diff++)
    {
        for (int y_diff = -1; y_diff <= 1; y_diff++)
        {
            if (x_diff != 0 || y_diff != 0)
            {
                if (get_wall(x + x_diff, y + y_diff))
                    neighbours++;
            }
        }
    }

    int neighbours_x = 0;
    int neighbours_y = 0;
    for (int i = 0; i < DIRECTIONS; i++)
    {
        if (get_wall(x + dir_x(i), y + dir_y(i)))
        {
            if (dir_x(i) != 0)
                neighbours_x++;
            else
                neighbours_y++;
        }
    }

    if ((neighbours >= 1 && (neighbours_x + neighbours_y) % 2 == 0
                && neighbours_x == neighbours_y)
            || ((neighbours_x + neighbours_y) % 2 == 1
                && neighbours % 2 == 1
                && neighbours - neighbours_x - neighbours_y > 2))
        return true;

    return false;
}

void add_node(vector<Node>& nodes, int x, int y)
{
    Node new_node;
    new_node.x = x;
    new_node.y = y;
    nodes.push_back(new_node);
}

void World::find_nodes(vector<Node>& nodes, int start_x, int start_y,
        int goal_x, int goal_y)
{
    add_node(nodes, start_x, start_y);
    add_node(nodes, goal_x, goal_y);
    for (int x = 1; x < cols - 1; x++)
    {
        for (int y = 1; y < rows - 1; y++)
        {
            if (is_node(x, y) && (x != start_x || y != start_y)
                    && (x != goal_x || y != goal_y))
                add_node(nodes, x, y);
        }
    }
}

void World::add_path_nodes(Node& a, Node& b, vector<int>& path)
{
    int added = 0;
    bool success = true;
    for (int x = a.x; x != b.x; x += 1 - 2 * (a.x > b.x))
    {
        if (get_wall(x, a.y))
        {
            success = false;
            break;
        }
        else if (x != a.x)
        {
            path.push_back(item(x, a.y));
            added++;
        }
    }
    if (success)
    {
        if (a.x != b.x)
            path.push_back(item(b.x, a.y));
        added++;
        for (int y = a.y; y != b.y; y += 1 - 2 * (a.y > b.y))
        {
            if (get_wall(b.x, y))
            {
                success = false;
                break;
            }
            else if (y != a.y)
            {
                path.push_back(item(b.x, y));
                added++;
            }
        }
    }
    if (success)
    {
        if (a.y != b.y)
            path.push_back(item(b.x, b.y));
        return;
    }

    path.resize(path.size() - added);
    for (int y = a.y; y != b.y; y += 1 - 2 * (a.y > b.y))
    {
        if (y != a.y)
            path.push_back(item(a.x, y));
    }
    if (a.y != b.y)
        path.push_back(item(a.x, b.y));
    for (int x = a.x; x != b.x; x += 1 - 2 * (a.x > b.x))
    {
        if (x != a.x)
            path.push_back(item(x, b.y));
    }
    if (a.x != b.x)
        path.push_back(item(b.x, b.y));
}

void World::convert_path_nodes(vector<Node>& nodes, vector<int>& indices,
        vector<int>& path)
{
    if (indices.size() > 1)
    {
        for (size_t i = 0; i < indices.size() - 1; i++)
            add_path_nodes(nodes[indices[i]], nodes[indices[i + 1]], path);
    }
}

void optimize_frontier(vector<AStarNode>& frontier)
{
    for (size_t i = 0; i < frontier.size() - 1; i++)
    {
        for (size_t j = 0; j < frontier.size(); j++)
        {
            if (i != j && frontier[j].path.back() == frontier[i].path.back()
                    && (frontier[j].cost > frontier[i].cost
                     || (frontier[j].cost == frontier[i].cost
                         && frontier[j].dist >= frontier[i].dist)))
            {
                frontier.erase(frontier.begin() + j);
                if (j < i)
                    i--;
            }
        }
    }
}

void World::find_path(int start_x, int start_y, int goal_x, int goal_y,
        vector<int>& path)
{
    int start = 0;
    int goal = 1;

    vector<Node> nodes;
    find_nodes(nodes, start_x, start_y, goal_x, goal_y);

    vector<AStarNode> frontier;
    add_a_star_node(frontier, nodes, start, goal);
    while (frontier.size() > 0)
    {
        size_t least;
        least = find_least(frontier);
        if (frontier[least].dist == 0)
        {
            convert_path_nodes(nodes, frontier[least].path, path);
            return;
        }
        add_a_star_connected_nodes(frontier, nodes, least);

        frontier.erase(frontier.begin() + least);
        optimize_frontier(frontier);
    }
}

enum MazeItem
{
    NOT,
    PASSAGE,
    WALL
};

void World::generate_maze()
{
    vector<int> maze(cols * rows, NOT);
    vector<int> frontier(1, item(robot->get_x(), robot->get_y()));
    while (frontier.size() > 0)
    {
        int r = rand() % frontier.size();
        if (maze[frontier[r]] == NOT)
        {
            int neighbours = 0;
            int walls = 0;
            for (int i = 0; i < DIRECTIONS; i++)
            {
                int n = item(get_x(frontier[r]) + dir_x(i),
                        get_y(frontier[r]) + dir_y(i));
                if (get_x(n) >= 1 && get_x(n) < cols - 1 && get_y(n) >= 1
                        && get_y(n) < rows - 1)
                {
                    if (maze[n] == NOT)
                        frontier.push_back(n);
                    else if (maze[n] == PASSAGE)
                        neighbours++;
                    for (int j = 0; j < DIRECTIONS; j++)
                    {
                        int m = item(get_x(n) + dir_x(j), get_y(n) + dir_y(j));
                        if (get_x(m) >= 1 && get_x(m) < cols - 1
                                && get_y(m) >= 1 && get_y(m) < rows - 1)
                        {
                            if (maze[m] == WALL)
                                walls++;
                        }
                    }
                }
            }
            if (neighbours > 1 && walls < DIRECTIONS - 1)
                maze[frontier[r]] = WALL;
            else
                maze[frontier[r]] = PASSAGE;
        }
        frontier.erase(frontier.begin() + r);
    }

    for (size_t i = 0; i < maze.size(); i++)
    {
        if (get_x(i) >= 1 && get_x(i) < cols - 1 && get_y(i) >= 1
                && get_y(i) < rows - 1)
            set_wall(i, maze[i] == WALL);
    }
}
