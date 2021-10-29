#include <stdio.h>

int main()
{
    char str[100] = {0}; char* a;
    sprintf(str, "%.8x", 0x71b29ad5);
    printf("%s\n", str);
    for (a = str; *a != '\0' ; a++) {
        printf("%x ", *a);
    }

    return 0;
}

