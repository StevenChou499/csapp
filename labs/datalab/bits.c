/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  /* Using products of sum and demorgan theorem */
  return (~(x & y) & ~(~x & ~y));
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /* Tmin's binary represents as 10000.....(30 zeros) in 2's complement */
  return (1 << 31);
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  /* Tmax + 1 == 0x80000000, which means () */
  return !~(((!~x) + ~0) & ((x + 1) ^ x));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /* If all odd bits are 1, x anding odd_mask makes odd_mask, 
   * and odd_mask ^ odd_mask makes 0 */
  int odd_mask = 0xAA | (0xAA << 8);
  odd_mask = odd_mask | (odd_mask << 16);
  return !((x & odd_mask) ^ odd_mask);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  /* By flipping all the bits and add by 1 */
  return ~(x) + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /* If x is an ascii digit, x minus 0x30 will give a non-negative result,
   * at the same time, x minus 0x40 will give a negative result.
   */
  return !((x + ~0x2F) >> 31 | ~(x + ~0x39) >> 31);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /* By utilizing !(x) + ~0 makes 0xFFFFFFFF or 0x00000000,
   * which will make (y ^ z) ^ 0 ^ z or (y ^ z) ^ y ^ 0, 
   * returns only y or z.
   */
  return (y ^ z) ^ ((!(x) + ~0) & z) ^ ((!!(x) + ~0) & y);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  /* We first fit x_s and y_s all with x's and y's sign bit. Then
   * we use (x_s ^ y_s) by check if x and y are the same sign. If 
   * they are the same sign, just check the subtraction result sign 
   * bit. If they are not the same sign, just check the x's sign bit.
   */
  int x_s = x >> 31;
  int y_s = y >> 31;
  return ~(((x_s ^ y_s) & (x_s)) | (~(x_s ^ y_s) & ((x + (~y)) >> 31))) + 1;
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  /* By shifting half of the remaining bytes to the upper part and 
   * implement bitwise and for 5 times (2^5 = 32), we can get the 
   * bitwise and result of all the 32 bits. If mask_1 is 0 means all
   * the bits are zeros.
   */
  int mask_16 = x | (x << 16);
  int mask_8 = mask_16 | (mask_16 << 8);
  int mask_4 = mask_8 | (mask_8 << 4);
  int mask_2 = mask_4 | (mask_4 << 2);
  int mask_1 = mask_2 | (mask_2 << 1);
  return (mask_1 >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  int bits = 0;
  int check_16;
  int check_8;
  int check_4;
  int check_2;
  int check_1;
  int if_shift_and_add;
  int diff = !((x ^ (x >> 1)) + ~0) & 1;
  check_16 = (x ^ (x >> 16)) >> 16;
  if_shift_and_add = (!check_16 + ~0) & 16;
  x >>= if_shift_and_add;
  bits += if_shift_and_add;
  check_8 = (x ^ (x >> 8)) >> 8;
  if_shift_and_add = (!check_8 + ~0) & 8;
  x >>= if_shift_and_add;
  bits += if_shift_and_add;
  check_4 = (x ^ (x >> 4)) >> 4;
  if_shift_and_add = (!check_4 + ~0) & 4;
  x >>= if_shift_and_add;
  bits += if_shift_and_add;
  check_2 = (x ^ (x >> 2)) >> 2;
  if_shift_and_add = (!check_2 + ~0) & 2;
  x >>= if_shift_and_add;
  bits += if_shift_and_add;
  check_1 = (x ^ (x >> 1)) >> 1;
  if_shift_and_add = (!check_1 + ~0) & 1;
  x >>= if_shift_and_add;
  bits += if_shift_and_add;
  return bits + !!(bits) + 1 + diff;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  /* If uf is Inf, Nan or 0, then directly return itsetlf.
   * If the exp field is all 0s, mean it's a denormalized number,
   * we only need to shift the exp and mantissa to the left by 1 bit.
   * If uf is normalized number, we only need to add the exp by 1.
   */
  unsigned sign = uf & 0x80000000;
  unsigned exp = uf & 0x7F800000;
  unsigned mantissa = uf & 0x007FFFFF;
  if (exp == 0x7F800000 || uf == 0x00000000) // Infinity or NaN or 0
    return uf;
  if (exp == 0x00000000) // denormalized
  {
    uf <<= 1;
    uf = (uf & 0x7FFFFFFF) | sign;
  }
  else // normalized
  {
    exp = (((exp >> 23) + 1) << 23) & 0x7F800000;
    uf = sign | exp | mantissa;
  }
  return uf;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  /* First check is the exp field is all zero or 1 then return 0 or
   * 0x8000000u. Then we shift the exp field by 23 and minus by 150.
   * We choose 150 is because when exp is 150, the represented floating
   * point value will be same as the mantissa itself. By checking if the 
   * exp is positive or negative, we shift the mantissa by left or right 
   * untill exp is zero. If the shifted mantissa is already zero, mean the 
   * result is already out of range (too many left shift) or too small for 
   * integer to represent (too many right shift).
   */
  unsigned sign = uf & 0x80000000;
  unsigned exp = uf & 0x7F800000;
  int exp_s = uf & 0x7F800000;
  unsigned mantissa = uf & 0x007FFFFF;
  if (exp == 0x00000000) // uf is zero or denormalized
    return 0;
  if (exp == 0x7F800000) // uf is Inf, Nan
    return 0x80000000u;
  exp_s = exp_s >> 23;
  exp_s -= 150; // 127 (bias) + 23 (mantissa bits) = 150
  // The value of mantissa is now when exp is 23
  mantissa |= 0x00800000;
  if (exp_s > 0) {
    while (exp_s--) { // left shift untill exp is zero
      mantissa <<= 1;
      if (mantissa == 0)
        return 0x80000000u;
    }
  }
  else {
    while (exp_s++) { // right shift untill exp is zero
      mantissa >>= 1;
      if (mantissa == 0)
        return 0;
    }
  }
  if (sign)
    return ~(mantissa) + 1;
  else
    return mantissa;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  /* If x is zero then return 2.
   * If x is larger than 127, adding bias 127 will be greater than 254, which 
   * can't be represented by float.
   * If x is smaller than -149, becase the smalled number float can represent 
   * is 2 ^ -149, so float can't represent.
   */
  if (x == 0) // return float representation of 2.0
    return 0x3f800000;
  if (x > 127) // too big to represent, return +Inf
    return 0x7F800000;
  if (x < -149) // too small to represent, return 0
    return 0;
  if (x > -127) {// -127 < x < 0, normalized
    return (x + 127) << 23;
  }
  else { // -149 < x <= -127, denormalized
    return 0x00800000 >> (127 - x);
  }
}
