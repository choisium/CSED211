#include <stdio.h>

long cread(long *xp) {
    return (xp ? *xp : 0);
}

long cread_alt(long *xp) {
    long zero = 0;
    long **xpp = &xp;
    return *(*xpp? *xpp : &zero);
}

int main() {
    long a = 1;
    printf("%ld %ld\n", cread(&a), cread_alt(&a));
    printf("%ld %ld\n", cread(NULL), cread_alt(NULL));
    return 0;
}
