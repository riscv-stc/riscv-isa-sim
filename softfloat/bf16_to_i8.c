
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

int_fast8_t bf16_to_i8( bfloat16_t a, uint_fast8_t roundingMode, bool exact )
{
  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool sign;
  int_fast16_t exp;
  uint_fast16_t frac;
  int_fast32_t sig32;
  int_fast16_t shiftDist;

  /*
   * get the signal, exp and fraction bits from the brain float number
   */
  uA.f = a;
  uiA = uA.ui;
  sign = signBF16UI( uiA );
  exp = expBF16UI( uiA );
  frac = fracBF16UI( uiA );

  /*
   *if NaN or infinite number
   */
  if ( 0xFF == exp )
  {
    softfloat_raiseFlags( softfloat_flag_invalid );

    return frac ? i8_fromNaN 
                : sign ? i8_fromNegOverflow : i8_fromPosOverflow;
  }

  /*
   * shift the frac bits to make the exp and frac bits together
   */
  sig32 = frac;
  //if normal number 
  if( exp )
  {
    //1+frac 
    sig32 |= 0x0080;

    //if no decimal; 0x86= 127(bias) + 7(frac bit number)
    //sig32 will exceed the biggest int8 value
    shiftDist = exp - 0x86;
    if( 0 <= shiftDist )
    {

      if( ( 0x80 == sig32 ) && ( 1 == sign ) && ( 0 == shiftDist ) )
      {//if -128
        return -128;
      }
      else
      {//overflow
        softfloat_raiseFlags( softfloat_flag_invalid );
        return sign ? i8_fromNegOverflow : i8_fromPosOverflow;        
      }

    }

    // 0x7A = 127(bias) - 5 
    //5 make the sig32 low 12bits are decimal bits
    //if shiftDist <= 0, the number is  too low
    shiftDist = exp - 0x7A;
    if ( 0 < shiftDist )
    {
      sig32 <<= shiftDist;
    }


  }

  
  return softfloat_roundToI8( 
    sign, (uint_fast32_t) sig32, roundingMode, exact );

}