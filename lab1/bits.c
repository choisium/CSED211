/* CSED 211 Fall '2021.  Lab L1 */
#include <limits.h>
#include <stdio.h>


#if 0
LAB L1 INSTRUCTIONS:

#endif

/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 */
int bitAnd(int x, int y) {
    return ~(~x | ~y);
}

/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 */
int addOK(int x, int y) {
    int check = x ^ (x + y);
    check |= (check >> 1) & (~((1 << 31) >> (1 + ~1 + 1)));
    check |= (check >> 2) & (~((1 << 31) >> (2 + ~1 + 1)));
    check |= (check >> 4) & (~((1 << 31) >> (4 + ~1 + 1)));
    check |= (check >> 8) & (~((1 << 31) >> (8 + ~1 + 1)));
    check |= (check >> 16) & (~((1 << 31) >> (16 + ~1 + 1)));
    check ^= (check >> 1) & (~((1 << 31) >> (1 + ~1 + 1))) ;
    int res = check & x;
    return !res;
}

/* 
 * isNegative - return 1 if x < 0, return 0 otherwise 
 *   Example: isNegative(-1) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int isNegative(int x) {
    return (x >> 31) & 1;
}

/* 
 * logicalShift - logical right shift of x by y bits, 1 <= y <= 31
 *   Example: logicalShift(-1, 1) = TMax.
 *   Legal ops: ! ~ & ^ | + << >>
 */
int logicalShift(int x, int y)
{
    return (x >> y) & (~((1 << 31) >> (y + ~1 + 1)));
}


/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 */
int bitCount(int x) {
    int test_bit = 1 | (1 << 8) | (1 << 16) | (1 << 24);  // 0x01010101, 6 ops

    // calculate count of 1 in every 8 bits
    int count = x & test_bit;  //  count of 1 for last 1 bit, 1 ops
    count += (x >> 1) & test_bit;  // count of 1 for last 2 bits, 3 ops
    count += (x >> 2) & test_bit;  // count of 1 for last 3 bits, 3 ops
    count += (x >> 3) & test_bit;  // count of 1 for last 4 bits, 3 ops
    count += (x >> 4) & test_bit;  // count of 1 for last 5 bits, 3 ops
    count += (x >> 5) & test_bit;  // count of 1 for last 6 bits, 3 ops
    count += (x >> 6) & test_bit;  // count of 1 for last 7 bits, 3 ops
    count += (x >> 7) & test_bit;  // count of 1 for last 8 bits, 3 ops

    // get sum of 1 for all bits by adding count in every 8 bits
    return (count + (count >> 8) + (count >> 16) + (count >> 24)) & 0xff;  // 7 ops
}

int main() {
    printf("%d\n", bitAnd(6, 5)); // 4
    printf("%d, %d\n", addOK(0x80000000,0x80000000), addOK(0x80000000,0x70000000)); // 0, 1
    printf("%d\n", isNegative(-1)); // 1
    printf("%X %X %d\n", -6, logicalShift(-6, 2), logicalShift(3, 1)); // 3FFFFFFE 1
    printf("%d %d\n", bitCount(5), bitCount(7)); // 2, 3

}