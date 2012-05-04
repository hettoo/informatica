#include <cmath>

#include "example1.h"

typedef enum X
{
    Y,
    Z
};

double bottom(int& reference)
{
    X test = X(0);
    return 1.0;
}

void repeatedly_called()
{
    int x = sin(5) + declared_but_not_defined_here(5);
    bottom(x);
}

void with_optional_arg(int n = 0)
{
}

void overloaded(int a, int b){
    repeatedly_called();
    with_optional_arg();
}

void overloaded(int a)
{
    overloaded(a, 7);
}

void recursion()
{
    function(3, 4);
    overloaded(6);
}

void nested(int array[])
{
    recursion();
}

double function(int a, int b)
{
    repeatedly_called();
    int a[5];
    nested(a);
    return a + b;
}

int recursive(int n)
{
    repeatedly_called();
    if (n > 0)
        n += recursive(n - 1);

    return n;
}

void procedure()
{
    recursive(5);
    recursive(5);
    recursive(5);
}

int main(int argc, char* argv[])
{
    procedure();
    function(1, 2);
    repeatedly_called();
}
