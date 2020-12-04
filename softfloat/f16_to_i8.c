
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

int_fast8_t f16_to_i8( float16_t a, uint_fast8_t roundingMode, bool exact )
{
  union ui16_f16 uA;
  uint_fast16_t uiA;
  bool sign;
  int_fast8_t exp;
  uint_fast16_t frac;
  int_fast32_t sig32;
  int_fast8_t sig8;
  int_fast8_t shiftDist;
  int_fast8_t result;

  /*
   * get the signal, exp and fraction bits from the float number
   */
  uA.f = a;
  uiA = uA.ui;
  sign = signF16UI( uiA );
  exp = expF16UI( uiA );
  frac = fracF16UI( uiA );

  /*
   *if NaN or infinite number
   */
  if ( 0x1F == exp )
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
    sig32 |= 0x0400;

    //if no decimal; 0x19= 15(bias) + 10(frac bit number)
    shiftDist = exp - 0x19;
    if( 0 <= shiftDist )
    {
      sig32 <<= shiftDist;

      //overflow handle 8(int8)
      if ( sig32 & UINT32_C( 0xFFFFFF80 ) ) 
      { 
        softfloat_raiseFlags( softfloat_flag_invalid );
        return sign ? i8_fromNegOverflow : i8_fromPosOverflow;
      }
      
      //get the integer 7bits and signal bit
      sig8 = sig32 & 0x7F;
      return sign ? -sig8 : sig8;
    }

    // 0x0D = 15(bias) - 2 
    //2 make the sig32 low 12bits is decimal bits, which is used in softfloat_roundToI8
    //if shiftDist <= 0, the number is  lower than 0.5 no influence to round
    shiftDist = exp - 0x0D;
    if ( 0 < shiftDist )
    {
      sig32 <<= shiftDist;
    }


  }

  
  return softfloat_roundToI8( 
    sign, (uint_fast32_t) sig32, roundingMode, exact );

}