#include<stdio.h>

void getbuf(){
    char buf[16];
    
    scanf ("%s", buf);
}

void smoke(){
    printf("smoke called\n");
}

int main()
{
    getbuf();
    printf("successfully finished\n");
    return 0;
}

