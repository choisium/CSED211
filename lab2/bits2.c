/* CSED 211 Fall '2021.  Lab L2 */


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
  unsigned sign = (uf >> 31) & 1;
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
    int i;
    for (i = g_pos - 2; i >= 0; i--) {
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
  unsigned sign = (uf >> 31) & 1;
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
  unsigned sign = (uf >> 31) & 1;
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
  unsigned sign = (uf >> 31) & 1;
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