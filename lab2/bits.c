/* CSED 211 Fall '2021.  Lab L2 */

#include <stdio.h>
#include <limits.h>
#include <math.h>

#if 0
FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict compared to the previous homework.  
You are allowed to use looping and conditional control.  
You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.

#endif

/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = (uf >> 23) & 0xFF;
  unsigned frac = uf & 0x7FFFFF;

  // if uf is NaN, return uf
  if ((exp == 0xff) && frac) return uf;

  // return -uf
  return (1 << 31) ^ uf;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  if (x == 0) return 0;
  unsigned sign, exp, frac;
  sign = (x >> 31) & 1;
  if (sign) x = -x;

  // find first 1 position
  int leftmost_one_pos;
  for (leftmost_one_pos = 31; leftmost_one_pos >= 0; leftmost_one_pos--) {
    if ((x >> leftmost_one_pos) & 1) break;
  }

  // compute exp bits
  exp = leftmost_one_pos + 127;

  // compute frac bits
  int g_pos = leftmost_one_pos - 23;
  int g = g_pos >= 0? (x >> g_pos) & 1 : 0;
  int r = g_pos >= 1? (x >> (g_pos - 1)) & 1 : 0;
  int s;
  if (g_pos >= 2) {
    s = x >> (g_pos - 2) & 1;
    for (int i = g_pos - 2; i >= 0; i--) {
      s |= ((x >> i) & 1);
    }
  } else {
    s = 0;
  }

  if (g_pos <= 0) { // don't need rounding
    frac = (x << (23 - leftmost_one_pos)) ^ (1 << 23);
  } else if (!r) { // don't need rounding
    frac = (x >> (leftmost_one_pos - 23)) ^ (1 << 23);
  } else if (r && (s || g)) { // need rounding
    frac = ((x >> (leftmost_one_pos - 23)) ^ (1 << 23)) + 1;
    if ((frac >> 23) & 1) { // need rounding again
      exp += 1;
      frac = frac << 1;
    }
  } else { // don't need rounding
    frac = (x >> (leftmost_one_pos - 23)) ^ (1 << 23);
  }
  frac &= 0x7FFFFF;
  return (sign << 31) | (exp << 23) | frac;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = (uf >> 23) & 0xFF;
  unsigned frac = uf & 0x7FFFFF;

  // if uf is NaN or already infinite, return uf
  if (exp == 0xff) return uf;
  // 2 * uf becomes overflow
  if (exp + 1 == 0xff) return (sign << 31) | ((exp + 1) << 23);
  // denormalized case
  if (exp == 0) return (sign << 31) | (frac << 1);

  return (sign << 31) | ((exp + 1) << 23) | frac;
}

/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = (uf >> 23) & 0xFF;
  unsigned frac = uf & 0x7FFFFF;

  // if uf is NaN, return uf
  if ((exp == 0xff) && frac) return uf;

  // return absolute value
  return (exp << 23) | frac;
}
/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Rating: 4
 */
unsigned float_half(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = (uf >> 23) & 0xFF;
  unsigned frac = uf & 0x7FFFFF;

  // if uf is NaN or already infinite, return uf
  if (exp == 0xff) return uf;
  // can divide by 2 only modifying exp
  if (exp > 1) {
    return (sign << 31) | ((exp - 1) << 23) | frac;
  }

  if ((frac & 1) && ((frac >> 1) & 1)) {
    // need rounding
    frac = (((exp << 23) | frac) >> 1) + 1;
  } else {
    // don't need rounding
    frac = ((exp << 23) | frac) >> 1;
  }
  return (sign << 31) | frac;
}


float u2f(unsigned x) {
  return *(float*) &x;
}

unsigned f2u(float x) {
  return *(unsigned*) &x;
}

int main() {
  // zero
  unsigned pzero = 0x00000000;
  unsigned nzero = 0x80000000;
  // denormalized
  unsigned pden = 0x00000001;
  unsigned nden = 0x80000001;
  // normalized
  unsigned pnor = 0x00800001;
  unsigned nnor = 0x80800001;
  // infinite
  unsigned pinf = 0x7F800000;
  unsigned ninf = 0xFF800000;
  // NaN
  unsigned pnan = 0x7F800001;
  unsigned nnan = 0xFF800001;
  // test integer
  int one = INT_MIN;
  // int rnd = 0x40000041;

  printf("float neg - nan: %X %X\n", float_neg(pnan), float_neg(nnan));
  printf("float neg - inf: %X %X\n", float_neg(pinf), float_neg(ninf));

  // printf("\n");
  printf("float neg\n");
  for (int i = pinf; i >= 0; i--) {
    if (i % 100000000 == 0) printf("float neg %d\n", i);
    if (f2u(-u2f(i)) != float_neg(i)) {
      printf("float neg - %d %X %X\n", i, f2u(-u2f(i)), float_neg(i));
      break;
    }
  }
  printf("plus done\n");
  for (int i = ninf; i <= 0; i--) {
    if (i % 100000000 == 0) printf("float neg %d\n", i);
    if (f2u(-u2f(i)) != float_neg(i)) {
      printf("float neg - %d %X %X\n", i, f2u(-u2f(i)), float_neg(i));
      break;
    }
  }
  printf("float neg done\n\n");

  printf("float i2f\n");
  for (int i = INT_MAX; i >= 0; i--) {
    // if (i % 100000000 == 0) printf("float i2f %d\n", i);
    if (f2u((float) i) != float_i2f(i)) {
      printf("float i2f - %d %X %X\n", i, f2u((float) i), float_i2f(i));
      break;
    }
  }
  for (int i = INT_MIN; i <= 0; i++) {
    // if (i % 100000000 == 0) printf("float i2f %d\n", i);
    if (f2u((float) i) != float_i2f(i)) {
      printf("float i2f - %d %X %X\n", i, f2u((float) i), float_i2f(i));
      break;
    }
  }
  printf("float i2f done\n\n");
  printf("float twice\n");
  for (int i = pinf; i >= 0; i--) {
    // if (i % 100000000 == 0) printf("float twice %d\n", i);
    if (f2u(2 * u2f(i)) != float_twice(i)) {
      printf("float twice - %d %X %X\n", i, f2u(2 * u2f(i)), float_twice(i));
      break;
    }
  }
  printf("plus done\n");
  for (int i = ninf; i <= 0; i--) {
    // if (i % 100000000 == 0) printf("float twice %d\n", i);
    if (f2u(2 * u2f(i)) != float_twice(i)) {
      printf("float twice - %d %X %X\n", i, f2u(2 * u2f(i)), float_twice(i));
      break;
    }
  }
  printf("float twice done\n\n");

  printf("float abs done\n");
  for (int i = pinf; i >= 0; i--) {
    // if (i % 100000000 == 0) printf("float abs %d\n", i);
    if (f2u(fabs(u2f(i))) != float_abs(i)) {
      printf("float abs - %d %X %X\n", i, f2u(fabs(u2f(i))), float_abs(i));
      break;
    }
  }
  printf("plus done\n");
  for (int i = ninf; i <= 0; i--) {
    // if (i % 100000000 == 0) printf("float abs %d\n", i);
    if (f2u(fabs(u2f(i))) != float_abs(i)) {
      printf("float abs - %d %X %X\n", i, f2u(fabs(u2f(i))), float_abs(i));
      break;
    }
  }
  printf("float abs done\n\n");

  printf("float half\n");
  for (int i = pinf; i >= 0; i--) {
    // if (i % 100000000 == 0) printf("float half %d\n", i);
    if (f2u(u2f(i) / 2) != float_half(i)) {
      printf("float half wrong - %d %X %X\n", i, f2u(u2f(i) / 2), float_half(i));
      break;
    }
  }
  printf("plus done\n");
  for (int i = ninf; i <= 0; i--) {
    // if (i % 100000000 == 0) printf("float half %d\n", i);
    if (f2u(u2f(i) / 2) != float_half(i)) {
      printf("float half wrong - %X %X %X\n", f2u(u2f(i)), f2u(u2f(i)/ 2), float_half(i));
      break;
    }
  }
  printf("float half done!\n");
}