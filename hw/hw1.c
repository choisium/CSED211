#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

// typedef union {
//     uint32_t u;
//     float f;
// } u2f;

typedef unsigned float_bits;


void print_binary(int num, int size) {
    for (int i = size - 1; i >= 0; --i) {
        int result = num >> i & 1;
        printf("%d ", result);
    }
}

// void print_bit(float num) {
//     for (int i = sizeof(float) * 8 - 1; i >= 0; --i) {
//         int result = ((int) num) >> i & 1;
//         printf("%d", result);
//     }
//     printf("\n");
// }

void show_ieee754 (float f)
{
    union {
        float f;
        uint32_t u;
    } fu = { .f = f };
    int i = sizeof f * CHAR_BIT;

    printf ("  ");
    while (i--)
        printf ("%d ", (fu.u >> i) & 0x1);

    putchar ('\n');
    printf (" |- - - - - - - - - - - - - - - - - - - - - - "
            "- - - - - - - - - -|\n");
    printf (" |s|      exp      |                  mantissa"
            "                   |\n\n");
}


int bitCount(int x) {
    x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x & 0x0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f);
    x = (x & 0x00ff00ff) + ((x >> 8) & 0x00ff00ff);
    x = (x & 0x0000ffff) + ((x >> 16) & 0x0000ffff);
    return x;
}

int prob1 (int x) {
    int a = !(x & 0xff);
    int b = !(~x);
    int c = !!(x >> (sizeof(int) - 1) * 8);
    int d = !!(x & (x - 1));

    int a_x = bitCount(x & 0xff) == 0;
    int b_x = bitCount(x) == 32;
    int c_x = bitCount(x & 0xff000000) >= 1;
    int d_x = bitCount(x) >= 2;

    if (a != a_x) {
        printf("prob1 a wrong! %X %d %d\n", x, a, a_x);
        return -1;
    }
    if (b != b_x) {
        printf("prob1 b wrong! %X %d %d\n", x, b, b_x);
        return -1;
    }
    if (c != c_x) {
        printf("prob1 c wrong! %X %d %d\n", x, c, c_x);
        return -1;
    }
    if (d != d_x) {
        printf("prob1 d wrong! %X %d %d\n", x, d, d_x);
        return -1;
    }
    return 0;
}

int prob3(int x) {
    int a = (x << 4) + (x << 1) + x;
    int b = - (x << 3) - x;
    int c = (x << 6) - (x << 3) - (x << 1);
    int d = -(x << 7) + (x << 2) + (x << 1);

    int a_x = x * 19;
    int b_x = x * (-9);
    int c_x = x * 54;
    int d_x = x * (-122);
    if (a != a_x) {
        printf("prob3 a wrong! %X %d %d\n", x, a, a_x);
        return -1;
    }
    if (b != b_x) {
        printf("prob3 b wrong! %X %d %d\n", x, b, b_x);
        return -1;
    }
    if (c != c_x) {
        printf("prob3 c wrong! %X %d %d\n", x, c, c_x);
        return -1;
    }
    if (d != d_x) {
        printf("prob3 d wrong! %X %d %d\n", x, d, d_x);
        return -1;
    }
    return 0;
};

int prob4(int k, int j) {
    int a = (~1 + 1) << k;
    int b = ~((~1 + 1) << k) << j;

    printf("%X %X\n", a, b);
    return 0;
}


float_bits prob8(int x) {
    unsigned exp, frac;
    if (x < -149) {   // zero
        exp = 0;
        frac = 0;
    } else if (x < -126) {  // denormalized value
        exp = 0;
        frac = 1 << (149 + x);
    } else if (x < 128) {
        exp = x + 127;
        frac = 0;
    } else {
        exp = 255;
        frac = 0;
    }

    return (exp << 23) | frac;
}

float_bits prob9(float_bits f) {
    unsigned sign = f >> 31;
    unsigned exp = (f >> 23) & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    if (((exp ^ 0xff) == 0) && frac & 0xff) {
        return f;
    }
    return (exp << 23) | frac;
}

unsigned int prob10(float_bits f) {
    unsigned sign = f >> 31;
    unsigned exp = (f >> 23) & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    // printf("%X %X %X %X\n", sign, exp, frac, (exp - 127));

    if ((!(exp ^ 0xff)) && frac) { // NaN(when exp = 0xFF && frac != 0)
        // printf("Nan!\n");
        return 0x7FFFFFFF;
    }
    if ((exp | frac) && sign) {  // negative(including -inf, excluding -0)
        // printf("negative!\n");
        return 0x80000000;
    }
    // if (!exp) { // denormalization, less than 1. 
    //     // printf("denormalization - less than 1!\n");
    //     return 0;
    // }
    if (exp >> 7) {  // overflow occurs when exp > 127
        // printf("overflow! %X, %X\n", exp, (exp - 127) >> 31);
        return 0x7FFFFFFF;
    }
    if ((exp - 127) >> 31) {  // denormalization or normalization case, less than 1
        // printf("normalization - less than 1!!\n");
        return 0;
    }
    // printf("available\n");
    // printf("%X %d\n", (1 << 23 | frac), (exp - 127));
    if (exp - 127 < 23) {
        return (1 << 23 | frac) >> (23 - (exp - 127));
    }
    return (1 << 23 | frac) << (exp - 127);
}

float u2f(unsigned x) {
  return *(float*) &x;
}

unsigned f2u(float x) {
  return *(unsigned*) &x;
}

int main() {
    // printf("problem 1\n");
    // // for(int i = INT_MAX; i >= 0; i--) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (prob1(i)) break;
    // // }
    // // for(int i = INT_MIN; i <= 0; i++) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (prob1(i)) break;
    // // }
    // printf("problem 1 done!\n");

    // printf("problem 3\n");
    // // for(int i = INT_MAX; i >= 0; i--) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (prob3(i)) break;
    // // }
    // // for(int i = INT_MIN; i <= 0; i++) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (prob3(i)) break;
    // // }
    // printf("problem 3 done!\n");

    // printf("problem 4\n");
    // // prob4(3, 5);
    // printf("problem 4 done!\n");
    // // pass 4, 5 now...
    // printf("problem 5\n");
    // int x, y;
    // x = INT_MIN; y = 0;
    // printf("a - %d\n", (x < y) == (-x > -y));
    // // for(int i = INT_MAX; i >= 0; i--) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (prob3(i)) break;
    // // }
    // // for(int i = INT_MIN; i <= 0; i++) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (prob3(i)) break;
    // // }
    // printf("problem 5 done!\n");

    // unsigned pinf = 0x7F800000;
    // unsigned ninf = 0xFF800000;
    // printf("problem 8\n");

    // for (int i = -300; i <= 300; i++) {
    //     if (f2u(powf(2.0, i)) != prob8(i)) {
    //         printf("prob 8 wrong - %d %X %X\n", i, f2u(powf(2.0, i)), prob8(i));
    //         break;
    //     }
    // }
    // printf("problem 8 done\n");


    // printf("problem 9\n");
    // // for (int i = pinf; i >= 0; i--) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (f2u(fabs(u2f(i))) != prob9(i)) {
    // //     printf("prob 9 wrong - %d %X %X\n", i, f2u(fabs(u2f(i))), prob9(i));
    // //     break;
    // //     }
    // // }
    // // for (int i = ninf; i <= 0; i--) {
    // //     if (i % 100000000 == 0) printf("\t%d\n", i);
    // //     if (f2u(fabs(u2f(i))) != prob9(i)) {
    // //     printf("prob 9 wrong - %d %X %X\n", i, f2u(fabs(u2f(i))), prob9(i));
    // //     break;
    // //     }
    // // }
    // printf("problem 9 done\n");

    printf("problem 10\n");
    for (unsigned int i = 0; i < UINT_MAX; i++) {
        if (i % 100000000 == 0) printf("\t%u\n", i);
        unsigned sign = (i >> 31) & 1;
        unsigned exp = (i >> 23) & 0xff;
        unsigned frac = i & 0x7FFFFF;
        
        if (exp == 0xff && frac != 0) {
            if (prob10(i) != 0x7FFFFFFF) {
                printf("prob 10 wrong - %X %X %X\n", i, 0x80000000, prob10(i));
                break;
            }
        } else if ((exp | frac) && sign) {
            if (prob10(i) != 0x80000000) {
                printf("prob 10 wrong - %X %X %X\n", i, 0x7FFFFFFF, prob10(i));
                break;
            }
        } else if (exp & 0x80) {
            if (prob10(i) != 0x7FFFFFFF) {
                printf("prob 10 wrong - %X %X %X\n", i, 0x80000000, prob10(i));
                break;
            }
        } else if (((int) u2f(i)) != prob10(i)) {
            printf("prob 10 wrong - %X %f %X %X\n", i, u2f(i), (int) u2f(i), prob10(i));
            break;
        }
    }
    printf("problem 10 done\n");



    // printf("%d\n", bitCount(0x40000000));

    // int x = 3;
    // int k = 5;
    // int j = 4;

    // printf("\nproblem 1_C\n");
    // printf("%d\n", !!(x >> (sizeof(int)-1) * 8));  // 0
    // printf("%d\n", !!(0x10000000 >> (sizeof(int)-1) * 8));   // 1

    // printf("\nproblem 1_D\n");
    // printf("%d\n", !!(x&(x-1)));  // 1
    // printf("%d\n", !!(j&(j-1)));   // 0
    // printf("%d\n", !!(0&(0-1)));   // 0
    // printf("%d\n", !!(7&(7-1)));   // 1

    // printf("\nproblem 3\n");
    // printf("%d %d\n", x * 19, (x << 4) + (x << 1) + x);
    // printf("%d %d\n", x * (-9), - (x << 3) - x);
    // printf("%d %d\n", x * 54, (x << 6) - (x << 3) - (x << 1));
    // printf("%d %d\n", x * (-122), -(x << 7) + (x << 2) + (x << 1));

    // printf("\nproblem 4\n");
    // print_binary((~1 + 1) << k, sizeof(int) * 8);
    // printf("\n");
    // print_binary((~((~1 + 1) << k)) << j, sizeof(int) * 8);
    // printf("\n");

    // printf("\nproblem 5\n");
    // int a = 0, b = 0;
    // printf("%d\n", ~a+~b+1 == ~(a+b));

    // printf("\nproblem 8\n");
    // fpwr2(129);

    // printf("\nproblem 9\n");
    // // show_ieee754((int) ((float) 1.0) & -1);
    // float_bits c = -1 ^ 0x33;
    // print_binary(c, sizeof(int) * 8);
    // printf("\n");
    // print_binary(float_absval(c), sizeof(int) * 8);
    // printf("\n");
    // // printf("%d %d\n", c, (int) float_absval(c));

    // printf("\nproblem 10\n");
    // // unsigned int d = 0x80000000;
    // u2f u1;
    // // u1.u = 0xFF800001;
    // u1.u = 0x01700000;
    // // printf("%f\n", u1.f);
    // print_binary(float_f2i(u1.u), sizeof(int) * 8);
    // printf("\n");
    // print_binary((int) u1.f, sizeof(int) * 8); 
    // printf("\n");
}
