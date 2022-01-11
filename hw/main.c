#include <stdio.h>
int a = 1;
static int b = 3;
extern int c;

int main()
{
    int c = 4;
    sub();
    exit(0);
}