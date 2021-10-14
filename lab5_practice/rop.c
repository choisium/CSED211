#include<stdio.h>

int GadgetA(int a)
{
    return a+3;
}

int GadgetB(int b)
{
    return b*3;
}

int getbuf()
{
    char buf[16];

    scanf("%s",buf);
    return 1;
}

int main()
{
    char buf[128];
    if (getbuf() == 13)
      printf("rop success\n");
    
    printf("finished\n");
    return 0;
}


