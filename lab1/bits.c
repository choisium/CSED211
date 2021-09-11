/* CSED 211 Fall '2021.  Lab L1 */



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
    return ((x >> 31 & 1) ^ (y >> 31 & 1)) | !((x >> 31 & 1) ^ ((x+y) >> 31 & 1));
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
    return (x >> y) & ~((~1 + 1) << (32 + ~y + 1));
}


/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 */
int bitCount(int x) {
    int count = (x & 0x55555555) + (x >> 1 & 0x55555555);
    count = (count & 0x33333333) + (count >> 2 & 0x33333333);
    count = (count & 0x0f0f0f0f) + (count >> 4 & 0x0f0f0f0f);
    count = (count & 0x00ff00ff) + (count >> 8 & 0x00ff00ff);
    count = (count & 0x0000ffff) + (count >> 16 & 0x0000ffff);
    return count;
}