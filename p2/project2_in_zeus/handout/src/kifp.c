/* Only Edit This File
 * ------------------
 *  Name: RongLian Yuan
 *  GNumber: G01313261
 */

#include <stdio.h>
#include <stdlib.h>
#include "common_structs.h"
#include "common_functions.h"
#include "kifp.h"
/* bias = 2^(3-1) - 1 = 3
 */
#define bias 3
#define fullBits 32

// Feel free to add many Helper Functions, Consts, and Definitions!

// ----------Public API Functions (write these!)-------------------

kifp_t addKifp(kifp_t val1, kifp_t val2);
kifp_t subKifp(kifp_t val1, kifp_t val2);
kifp_t negateKifp(kifp_t value);

/*  Helper function. This functions is trying to convert the decimal number into binary value as a whole number.
 */
int transferToBinary(int myNumber)
{
  int binaryValue = 0, remainder = 0, times = 1;

  while (myNumber != 0)
  {
    remainder = myNumber % 2;
    binaryValue = binaryValue + remainder * times;
    myNumber = myNumber / 2;
    times = times * 10;
  }

  return binaryValue;
}
/*  Helper function. This function is trying to get the E. E is the exp part comes from a binary number with scientific notation.
    The algorithm is to get how many digits of a decimal. For example, 10101 with a 5 digits and E = 4;
*/
int getE(int binary)
{
  int E_value = 0;
  while (binary >= 1)
  {
    binary = binary / 10;
    E_value++;
  }
  return (E_value - 1);
}
/*  Helper function. This function is used when the whole number part is 0, we can only get the E from the
  fraction part. I only look to the first number of hex. The way of that is shift all the way
  right.
*/
int getNegativeE(int hexFrac)
{
  int E_Negative_value = 0;
  hexFrac = hexFrac >> (fullBits - 4);
  if (hexFrac == 0)
  {
    E_Negative_value = -5;
  }
  else if (hexFrac == 0x1)
  {
    E_Negative_value = -4;
  }
  else if (hexFrac == 0x2 || hexFrac == 0x3)
  {
    E_Negative_value = -3;
  }
  else if (hexFrac == 0x4 || hexFrac == 0x5 || hexFrac == 0x6 || hexFrac == 0x7)
  {
    E_Negative_value = -2;
  }
  else
  {
    E_Negative_value = -1;
  }

  return E_Negative_value;
}

/*  Helper function, help me to find frac part only. 0x01f = 0 0001 1111.
  It helps me delete the first four bits to 0 and remains the left 5 bits.
*/
int getFracPart(int value)
{
  int answer;
  int mask = 0x01f;
  answer = value & mask;

  return answer;
}

/*  Helper function, help me find exp part only. 0x0e0 = 0 1110 0000.
  right shift 5 bits means get Exp part only.
*/
int getExpPart(int value)
{
  int answer;
  int mask = 0x0e0;
  answer = value & mask;
  answer = answer >> 5;

  return answer;
}

/*  Helper function, help me find sign bit only.0x100 = 1 0000 0000.
  right shift 8 bits means get sign bit only.
*/
int getSignPart(int value)
{
  int answer;
  int mask = 0x100;
  answer = value & mask;
  answer = answer >> 8;
  return answer;
}

/*  Helper function, giving the exp part and get the E
  if expPart = (6, 5, 4, 3, 2, 1) -> E = (3, 2, 1, 0, -1, -2) normalized
  if expPart =  (0) -> E = (-2) de-normalized
*/
int getMyE(int expPart)
{
  int answer;
  if (expPart > 0)
  {
    answer = expPart - bias;
  }
  else
  {
    answer = 1 - bias;
  }

  return answer;
}

/*  Helper function that get the mantissa part of given exp and frac part.
 */
int getMantissaPart(int exp, int frac)
{
  int answer;
  /*  exp == 0: de-normalized, whole part = 0
      exp != 0: normalized, whole part = 1
  */
  if (exp == 0)
  {
    answer = frac;
  }
  else
  {
    answer = frac | (1 << 5);
  }

  return answer;
}

/*  Helper function that make adjustment  of E after multiple to make sure manstissa is under
  correct scientific notation. The binary point should be put between  bit 10 and bit 11, if there
  is a '1' or '0'. Adjustment needed.
    1 << 11: bit 12
    1 << 10: bit 11
    1 << 9: bit 10
    1 << 8: bit 9
    1 << 7: bit 8
    1 << 6: bit 7
    1 << 5: bit 6
  bit:       12 11. 10 9 8 7 6 5 4 3 2 1
  eg:
  before:    1  0 .  0 0 0 0 0 0 0 0 0 0
  after:     1. 0    0 0 0 0 0 0 0 0 0 0
*/
int adjustE(int mantissa)
{
  int adjustment;
  if (1 & (mantissa >> 11) == 1)
  {
    adjustment = 1;
  }
  else if (1 & (mantissa >> 10) == 1)
  {
    adjustment = 0;
  }
  else if (1 & (mantissa >> 9) == 1)
  {
    adjustment = -1;
  }
  else if (1 & (mantissa >> 8) == 1)
  {
    adjustment = -2;
  }
  else if (1 & (mantissa >> 7) == 1)
  {
    adjustment = -3;
  }
  else if (1 & (mantissa >> 6) == 1)
  {
    adjustment = -4;
  }
  else if (1 & (mantissa >> 5) == 1)
  {
    adjustment = -5;
  }

  return adjustment;
}

/*  Helper function, combine mansitssa and exp part
    finalE > 3 (4, 5, 6 ...) exp = E+bias = 7, 8 , 9 ... Exp part = 111 (inf) 0xe0 = 111 00000
    finalE > -2: normalized
        if adjustE = 1  -> get bit 11 to 7  mask = 0111 1100 0000 //0x7c0
        if adjustE = 0  -> get bit 10 to 6  mask = 0011 1110 0000 //0x3e0
        if adjustE = -1 -> get bit 9 to 5   mask = 0001 1111 0000 //0x1f0
        if adjustE = -2 -> get bit 8 to 4   mask = 0000 1111 1000 //0x0f8
        if adjustE = -3 -> get bit 7 to 3   mask = 0000 0111 1100 //0x07c
    E < -2: de-normalized
        exp = 000          get bit 10 to 6  mask = 0011 1110 0000//0x3e0
    E = -2:
        if bit 11 or 12 == 1 normalized      exp = finalS + bias
        if bit 11 == 0       de-normalized   exp = 000
*/
int adjust_combine(int addition_S1_S2, int adjustE, int mantissa)
{
  int answer, finalE, finalExp;
  finalE = addition_S1_S2 + adjustE;

  if (finalE > 3)
  {
    answer = 0xe0;
  }
  else if (finalE > -2)
  {

    finalExp = finalE + bias;

    if (adjustE == 1)
    {
      mantissa = (mantissa & 0x7c0) >> 6;
      answer = mantissa | (finalExp << 5);
    }
    else if (adjustE == 0)
    {
      mantissa = (mantissa & 0x3e0) >> 5;
      answer = mantissa | (finalExp << 5);
    }
    else if (adjustE == -1)
    {

      mantissa = (mantissa & 0x1f0) >> 4;

      answer = mantissa | (finalExp << 5);
    }
    else if (adjustE == -2)
    {
      mantissa = (mantissa & 0x0f8) >> 3;
      answer = mantissa | (finalExp << 5);
    }
    else if (adjustE == -3)
    {
      mantissa = (mantissa & 0x07c) >> 2;
      answer = mantissa | (finalExp << 5);
    }
  }
  else if (finalE < -2)
  {
    mantissa = (mantissa & 0x3e0) >> 5;
    finalExp = 0;
    answer = mantissa | (finalExp << 5);
  }
  else if (finalE == -2)
  {
    if (adjustE == 1)
    {
      mantissa = (mantissa & 0x7c0) >> 6;
      finalExp = finalE + bias;
      answer = mantissa | (finalExp << 5);
    }
    else if (adjustE == 0)
    {
      mantissa = (mantissa & 0x3e0) >> 5;
      finalExp = finalE + bias;
      answer = mantissa | (finalExp << 5);
    }
    else if (adjustE < 0)
    {
      mantissa = (mantissa & 0x3e0) >> 5;
      finalExp = 0;
      answer = mantissa | (finalExp << 5);
    }
  }

  return answer;
}

/*  Helper function that right shift bits of mantissa
 */
int adjustMyMantissa(int mantissa, int right_shift_bits)
{
  int answer;
  answer = mantissa >> right_shift_bits;

  return answer;
}

/*  Help function that combine mantissa part and exp part of addition
 */
int combine_addition(int mantissa, int exp)
{
  int answer, finalE, shiftE, ifNormalize = 1;
  if (mantissa >> 6 == 1)
  {
    shiftE = 1;
    mantissa = (mantissa & 0x03e) >> 1;
    ifNormalize = 1;
  }
  else if (mantissa >> 5 == 1)
  {
    shiftE = 0;
    mantissa = mantissa & 0x01f;
    ifNormalize = 1;
  }
  else
  {
    shiftE = 0;
    mantissa = mantissa & 0x01f;
    ifNormalize = 0;
  }

  finalE = exp + shiftE;
  if (finalE > 3)
  {
    answer = 0x0e0;
  }
  else if (finalE > -2)
  {
    finalE = finalE + bias;
    answer = mantissa | (finalE << 5);
  }
  else
  {
    if (ifNormalize == 1)
    {
      finalE = finalE + bias;
      answer = mantissa | (finalE << 5);
    }
    else
    {
      answer = mantissa;
    }
  }

  return answer;
}

/*  Helper function that can make sure the actually sign if add/substraction
  1.both positive
  2.both negative
  3.one positive/one negative
*/

int getSignFromDifferentSign(int sign1, int sign2, int val1, int val2)
{
  int answerSign;
  kifp_t positive_val1 = val1 & 0x0ff;
  kifp_t positive_val2 = val2 & 0x0ff;

  if (sign1 + sign2 == 2)
  {
    answerSign = 1 << 8;
  }
  else if (sign1 + sign2 == 0)
  {
    answerSign = 0 << 8;
  }
  else
  {
    if (sign1 == 1)
    {
      if (positive_val1 > positive_val2)
      {

        answerSign = 1 << 8;
      }
      else
      {
        answerSign = 0 << 8;
      }
    }
    else
    {
      if (positive_val1 < positive_val2)
      {
        answerSign = 1 << 8;
      }
      else
      {
        answerSign = 0 << 8;
      }
    }
  }
  return answerSign;
}

/*  Help function that combine mantissa part and exp part of substraction
 */
int combine_subtraction(int mantissa, int exp)
{

  int answer, shiftE, finalE;
  if (mantissa >> 9 == 1)
  {
    shiftE = 4;
    mantissa = (mantissa & 0x3f0) >> 4;
  }
  else if (mantissa >> 8 == 1)
  {
    shiftE = 3;
    mantissa = (mantissa & 0x1f8) >> 3;
  }
  else if (mantissa >> 7 == 1)
  {
    shiftE = 2;
    mantissa = (mantissa & 0x0fc) >> 2;
  }
  else if (mantissa >> 6 == 1)
  {
    shiftE = 1;
    mantissa = (mantissa & 0x07e) >> 1;
  }
  else if (mantissa >> 5 == 1)
  {
    shiftE = 0;
    mantissa = mantissa & 0x03f;
  }
  else if (mantissa >> 4 == 1)
  {
    shiftE = -1;
    mantissa = (mantissa << 1) & 0x03f;
  }
  else if (mantissa >> 3 == 1)
  {
    shiftE = -2;
    mantissa = (mantissa << 2) & 0x03f;
  }
  else if (mantissa >> 2 == 1)
  {
    shiftE = -3;
    mantissa = (mantissa << 3) & 0x03f;
  }
  else if (mantissa >> 1 == 1)
  {
    shiftE = -4;
    mantissa = (mantissa << 4) & 0x03f;
  }
  else if (mantissa >> 0 == 1)
  {
    shiftE = -5;
    mantissa = (mantissa << 5) & 0x03f;
  }

  finalE = exp + shiftE;

  if (finalE > 3)
  {
    answer = 0x0e0;
  }
  else if (finalE > -2)
  {
    finalE = finalE + bias;
    answer = (mantissa & 0x01f) | (finalE << 5);
  }
  else if (finalE == -2)
  {
    if (mantissa >> 5 == 1)
    {
      finalE = finalE + bias;
      answer = (mantissa & 0x01f) | (finalE << 5);
    }
    else
    {
      finalE = 0;
      answer = (mantissa & 0x01f) | (finalE << 5);
    }
  }
  return answer;
}

int adjustMyMantissaInSub(int mantissa, int left_shift_bits)
{
  int answer = mantissa << left_shift_bits;

  return answer;
}

// toKifp - Converts a Number Struct (whole and fraction parts) into a KIFP Value
// number is managed by zuon, DO NOT FREE number.
// Return the KIFP Value on Success or -1 on Failure.
kifp_t toKifp(Number_t *number)
{
  if (number == NULL)
  {
    return -1;
  }
  /*  Deal with infinity and NaN part
   */
  if (number->is_negative == 0 && number->is_infinity == 1)
  {
    return 0x0E0;
  }

  if (number->is_negative == 1 && number->is_infinity == 1)
  {
    return 0x1E0;
  }

  if (number->is_nan == 1)
  {
    return 0x0E1;
  }

  int wholeToBinary = transferToBinary(number->whole);
  int E = getE(wholeToBinary);
  /*  Overlapping of the exp part.
   */
  if (E > 3 && number->is_negative == 0)
  {
    return 0x0E0;
  }
  if (E > 3 && number->is_negative == 1)
  {
    return 0x1E0;
  }

  /* make it unsigned, or shift will make all '1s' if the leading bit is 1
   */
  unsigned int answer, newWholeValue, newExp, currentFraction;
  currentFraction = number->fraction;
  /*  NORMALIZED in which E >= 0
   */
  if (E >= 0)
  {
    answer = currentFraction >> E;
    newWholeValue = number->whole << (fullBits - E);
    answer = answer | newWholeValue;
    /*  5 is the start bit of frac part
     */
    answer = answer >> (fullBits - 5);
    /*  put the exponent to the corresponding place
     */
    newExp = (bias + E) << 5;
    answer = answer | newExp;
  }
  else if (E < 0)
  {
    E = getNegativeE(number->fraction);
    /*  NORMALIZED in which E > -3
     */
    if (E > -3)
    {
      answer = currentFraction << (-E);
      answer = answer >> (fullBits - 5);
      newExp = (bias + E) << 5;
      answer = answer | newExp;
    }
    else
    {
      /*  DE-NORMALIZED in which E > -3
        in which E = 1- bias and the EXP is all 0 (in which do not need to do shift or bitewise operation)
      */
      E = 1 - bias;
      answer = currentFraction << (-E);
      answer = answer >> (fullBits - 5);
    }
  }

  return answer;
}

// toNumber - Converts a KIFP Value into a Number Struct (whole and fraction parts)
// number is managed by zuon, DO NOT FREE or re-Allocate number. (It is already allocated)
// Return 0 on Success or -1 on Failure.
int toNumber(Number_t *number, kifp_t value)
{
  if (number == NULL || value == -1)
  {
    return -1;
  }
  /*  Right shif 8 bits to get the sign bit value. 3 for exp and 5 for frac.
   */
  number->is_negative = value >> (3 + 5);
  /*  The inf value of KIFP is 0x0E0 (whatever negative or positive)
   */
  if (value == 0x0E0 || value == 0x1E0)
  {
    number->is_infinity = 1;
  }
  else
  {
    number->is_infinity = 0;
  }
  /*  right shift 5 bits to see if the result equals to 7 (0111-> positive) or f(1111-> negative),
    In addition if shift it left 4 bits (remaining frac parts), if it is not 0, NaN
  */
  if ((value >> 5 == 7 || value >> 5 == 0xf) && value << 4 != 0)
  {
    number->is_nan = 1;
  }
  else
  {
    number->is_nan = 0;
  }

  /*  get the exponent  part
   */
  int expPart = getExpPart(value);

  /*  To get the frac part
   */
  int frac = getFracPart(value);

  int E;

  /* frac is a 5-bit value, left shift (32-5) bits to fill in 32 bits format
    if expPart = (6, 5, 4, 3) -> E = (3, 2, 1, 0) normalized, E >= 0: first shift left E bits in order to later
  delete unrelated bits which belongs to whole number.
    if expPart = (2, 1) -> E = (-1, -2) normalized, E < 0
    if expPart =  (0) -> E = (-2) de-normalized, E < 0
  */
  if (expPart < 7 && expPart > 2)
  {
    E = expPart - bias;
    frac = frac << E;
    number->fraction = frac << (fullBits - 5);

    /*  whole_part_in_frac is to figure out the whole number part in frac
      for example, 11.1001 -> 1.11001 * 2^1, in which E = 1, frac = 11001, thus,
      the first 1 in frac belongs to whole number. whole_part_2 is the original whole number.
    */
    frac = getFracPart(value);
    int whole_part_in_frac = frac >> (5 - E);
    int whole_part_2 = 1 << E;
    number->whole = (whole_part_in_frac | whole_part_2);
  }
  if (expPart < 3)
  {
    if (expPart > 0)
    {
      E = expPart - bias;
      frac = getFracPart(value);
      frac = frac | (1 << 5);
      if (E == -2)
      {
        number->fraction = frac << (fullBits - 7);
        number->whole = 0;
      }
      else
      {
        number->fraction = frac << (fullBits - 6);
        number->whole = 0;
      }
    }
    else
    {
      frac = getFracPart(value);
      number->fraction = frac << (fullBits - 7);
      number->whole = 0;
    }
  }
  return 0;
}

// mulKIFP - Multiplies two KIFP Values together using the Techniques from Class.
// - To get credit, you must work with S, M, and E components.
// - You are allowed to shift/adjust M and E to multiply whole numbers.
// Return the resulting KIFP Value on Success or -1 on Failure.
kifp_t mulKifp(kifp_t val1, kifp_t val2)
{
  if (val1 == -1 || val2 == -1)
  {
    return -1;
  }

  /*  get the sign part. S = S1 ^ S2
    if S = 0, positive, 0 000 00000
    if S = 1, negative, 1 000 00000
  */
  int sign1 = getSignPart(val1);
  int sign2 = getSignPart(val2);
  int answerSign = sign1 ^ sign2;

  answerSign = answerSign << 8;

  /*  Special cases
   */
  kifp_t positive_val1 = val1 & 0x0ff;
  kifp_t positive_val2 = val2 & 0x0ff;

  if (positive_val1 == 0x0e1 || positive_val2 == 0x0e1)
  {
    return 0x0e1;
  }

  if (positive_val1 == 0x0e0 || positive_val2 == 0x0e0)
  {
    if (positive_val1 == 0 || positive_val2 == 0)
    {
      return 0x0e1;
    }
    else
    {
      return (0x0e0 | answerSign);
    }
  }

  if (positive_val1 == 0 || positive_val2 == 0)
  {
    if (positive_val1 == 0x0e0 || positive_val2 == 0x0e0)
    {
      return 0x0e1;
    }
    else
    {
      return 0;
    }
  }

  /*  get the exponent  part
   */
  int expPart1 = getExpPart(val1);
  int expPart2 = getExpPart(val2);
  int E1 = getMyE(expPart1);
  int E2 = getMyE(expPart2);
  int answerExp = E1 + E2;

  /*  To get the frac part(5 bits from very right bit)
   */
  int frac1 = getFracPart(val1);
  int frac2 = getFracPart(val2);

  /*  To get the mantissa part and multiple them together
    if expPart =  000 ---> de-normalized -> add 0 before frac part
    if expPart != 000 ---> normalized    -> add 1 before frac part
  */
  int mantissa1 = getMantissaPart(expPart1, frac1);
  int mantissa2 = getMantissaPart(expPart2, frac2);
  int answerMantissa = mantissa1 * mantissa2;

  /*  adjustedE is the helper date help me track E
      eg:   0.015625 (1/64) *  1.90625 (61/32)
            0.0001 * 2^(-2)  *  1.11101 * 2^(0)
        ____________________________________________
                  0.000111101 * 2^(-2) -----> legal   ---> answerExp = -2
        adjusted:      1.111101 * 2^(-6) ---> illegal ---> adjustedE = -6
  */
  int adjustedE = adjustE(answerMantissa);
  int combine_E_Mantissa = adjust_combine(answerExp, adjustedE, answerMantissa);

  int finalAnswer = combine_E_Mantissa | answerSign;

  return finalAnswer;
}

// addKIFP - Adds two KIFP Values together using the Addition Techniques from Class.
// - To get credit, you must work with S, M, and E components.
// - You are allowed to shift/adjust M and E as needed.
// Return the resulting KIFP Value on Success or -1 on Failure.
kifp_t addKifp(kifp_t val1, kifp_t val2)
{
  if (val1 == -1 || val2 == -1)
  {
    return -1;
  }

  if (val1 == 0x0e1 || val2 == 0x0e1)
  {
    return 0x0e1;
  }
  if (val1 == 0x0e0 && val2 == 0x0e0)
  {
    return 0x0e0;
  }
  if (val1 == 0x0e0 || val2 == 0x0e0)
  {

    return 0x0e0;
  }

  /*  Get the sign bit of 9-bit-fp
      if both val1 and val2 are positive, sign1 = 0, sign2 = 0 -> addition is positive
      if both val1 and val2 are negative, sign1 = 1, sign2 = 1 -> addition is negative
      if one negative one positive, which absolute value is larger, which sign it will be
  */

  int sign1 = getSignPart(val1);
  int sign2 = getSignPart(val2);

  int answerSign = getSignFromDifferentSign(sign1, sign2, val1, val2);

  kifp_t positive_val1 = val1 & 0x0ff;
  kifp_t positive_val2 = val2 & 0x0ff;

  if (sign1 != sign2)
  {
    if (sign1 == 1)
    {
      val1 = val1 & 0x0ff;
    }
    else
    {
      val2 = val2 & 0x0ff;
    }
    if (positive_val1 > positive_val2)
    {
      int answer = subKifp(val1, val2);
      answer = answer | answerSign;
      return answer;
    }
    else
    {
      int answer = subKifp(val2, val1);
      answer = answer | answerSign;
      return answer;
    }
  }

  /*  Get the exp part of 9-bit-fp and calculate the E
   */
  int expPart1 = getExpPart(val1);
  int expPart2 = getExpPart(val2);
  int E1 = getMyE(expPart1);
  int E2 = getMyE(expPart2);
  int difference, adjustedE;
  if (E1 > E2)
  {
    difference = E1 - E2;
    adjustedE = E1;
  }
  else if (E1 < E2)
  {
    difference = E2 - E1;
    adjustedE = E2;
  }
  else
  {
    difference = 0;
    adjustedE = E1;
  }

  /*  Get the frac part of 9-bit-fp
   */

  int frac1 = getFracPart(val1);
  int frac2 = getFracPart(val2);

  /*  Get the mantissa
      Making adjustment.
      Normalize the smaller number to match the E of the larger number -> always shift right
      If E1 > E2, adjustment mantissa2
      If E2 > E1, adjustment mantissa1
      else, do not need to make adjustment
  */
  int mantissa1 = getMantissaPart(expPart1, frac1);
  int mantissa2 = getMantissaPart(expPart2, frac2);
  int answerMantissa, final_answer;

  if (E1 > E2)
  {
    mantissa2 = adjustMyMantissa(mantissa2, difference);
    answerMantissa = mantissa1 + mantissa2;
    final_answer = combine_addition(answerMantissa, E1);
  }
  else if (E1 < E2)
  {
    mantissa1 = adjustMyMantissa(mantissa1, difference);
    answerMantissa = mantissa1 + mantissa2;
    final_answer = combine_addition(answerMantissa, E2);
  }
  else
  {
    answerMantissa = mantissa1 + mantissa2;
    final_answer = combine_addition(answerMantissa, E1);
  }

  final_answer = final_answer | answerSign;

  return final_answer;
}

// subKIFP - Subtracts two KIFP Values together using the Addition Techniques from Class.
// - To get credit, you must work with S, M, and E components.
// - You are allowed to shift/adjust M and E as needed.
// Return the resulting KIFP Value on Success or -1 on Failure.
kifp_t subKifp(kifp_t val1, kifp_t val2)
{
  if (val1 == -1 || val2 == -1)
  {
    return -1;
  }
  if (val1 == val2)
  {
    return 0;
  }
  if (val1 == 0x0e1 || val2 == 0x0e1)
  {
    return 0x0e1;
  }
  if (val1 == 0x1e1 && val2 == 0x0e1)
  {
    return 0x1e1;
  }
  if (val1 == 0x0e0 || val2 == 0x0e1)
  {
    if (val1 != 0x0e0 || val2 != 0x0e1)
    {
      return 0x0e0;
    }
  }
  if (val1 == 0x1e0 || val2 == 0x1e1)
  {
    if (val1 != 0x1e0 || val2 != 0x1e1)
    {
      return 0x1e0;
    }
  }

  /*  val1 - val2 == val1 + (-val2).
    So make val2 to nagative first. and then make sure the sign.
  */
  int sign1 = getSignPart(val1);
  int to_positive_Val2 = val2 & 0x0ff;
  int sign2 = getSignPart(to_positive_Val2);
  int realSign2 = getSignPart(val2);

  int answerSign = getSignFromDifferentSign(sign1, sign2, val1, val2);
  if (sign1 != realSign2)
  {
    val1 = val1 & 0x0ff;
    val2 = val2 & 0x0ff;
    int answer = addKifp(val1, val2);
    answer = negateKifp(answer);
    return answer;
  }

  if (val1 == 0)
  {
    if (val2 == 0x1e0)
    {
      return 0x0e0;
    }
    else if (val2 == 0x0e0)
    {
      return 0x1e0;
    }
    else if (val2 == 0x0e1)
    {
      return 0x0e1;
    }
    else
    {
      return negateKifp(val2);
    }
  }

  /*  Get the exp part of 9-bit-fp and calculate the E
   */
  int expPart1 = getExpPart(val1);
  int expPart2 = getExpPart(val2);
  int E1 = getMyE(expPart1);
  int E2 = getMyE(expPart2);
  int difference, adjustedE;
  if (E1 > E2)
  {
    difference = E1 - E2;
    adjustedE = E1;
  }
  else if (E1 < E2)
  {
    difference = E2 - E1;
    adjustedE = E2;
  }
  else
  {
    difference = 0;
    adjustedE = E1;
  }

  /*  Get the frac part of 9-bit-fp
   */

  int frac1 = getFracPart(val1);
  int frac2 = getFracPart(val2);

  /*  Get the mantissa
      Making adjustment.
      Normalize the bigger number to match the E of the smaller number -> always shift left
      If E1 > E2, adjustment mantissa1
      If E2 > E1, adjustment mantissa2
      else, do not need to make adjustment
  */
  int mantissa1 = getMantissaPart(expPart1, frac1);
  int mantissa2 = getMantissaPart(expPart2, frac2);
  int answerMantissa, final_answer;

  if (E1 > E2)
  {
    mantissa1 = adjustMyMantissaInSub(mantissa1, difference);
    if (mantissa1 > mantissa2)
    {
      answerMantissa = mantissa1 - mantissa2;
      final_answer = combine_subtraction(answerMantissa, E2);
    }
    else
    {
      answerMantissa = mantissa2 - mantissa1;
      final_answer = combine_subtraction(answerMantissa, E2);
      final_answer = negateKifp(final_answer);
    }
  }
  else if (E1 < E2)
  {
    mantissa2 = adjustMyMantissaInSub(mantissa2, difference);

    if (mantissa1 > mantissa2)
    {
      answerMantissa = mantissa1 - mantissa2;
      final_answer = combine_subtraction(answerMantissa, E1);
    }
    else
    {
      answerMantissa = mantissa2 - mantissa1;
      final_answer = combine_subtraction(answerMantissa, E1);
      final_answer = negateKifp(final_answer);
    }
  }
  else
  {
    if (mantissa1 > mantissa2)
    {
      answerMantissa = mantissa1 - mantissa2;
      final_answer = combine_subtraction(answerMantissa, E1);
    }
    else
    {
      answerMantissa = mantissa2 - mantissa1;
      final_answer = combine_subtraction(answerMantissa, E1);
      final_answer = negateKifp(final_answer);
    }
  }
  final_answer = final_answer | answerSign;

  return final_answer; // Replace this Line with your Code
}

// negateKIFP - Negates a KIFP Value.
// Return the resulting KIFP Value on Success or -1 on Failure.
kifp_t negateKifp(kifp_t value)
{
  /*  8 is the start bit of sign
   */
  if (value == -1)
  {
    return -1;
  }

  kifp_t myAnswer;
  int sign = value >> 8;
  if (sign == 1)
  {
    myAnswer = value & 0x0ff;
  }
  else
  {
    myAnswer = value | (1 << 8);
  }

  return myAnswer; // Replace this Line with your Code
}
