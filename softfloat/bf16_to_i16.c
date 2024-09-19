
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

int_fast16_t bf16_to_i16( bfloat16_t a, uint_fast8_t roundingMode, bool exact )
{
  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool sign;
  int_fast16_t exp;
  uint_fast16_t frac;
  int_fast32_t sig32;
  int_fast16_t sig16;
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

    return frac ? i16_fromNaN 
                : sign ? i16_fromNegOverflow : i16_fromPosOverflow;
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
    shiftDist = exp - 0x86;
    if( 0 <= shiftDist )
    {

      if( 8 <= shiftDist )
      {
        if( (0x80 == sig32) && ( 8 == shiftDist ) && ( 1 == sign) )
        {
          return -0x8000;
        }
        else
        {
          softfloat_raiseFlags( softfloat_flag_invalid );
          return sign ? i16_fromNegOverflow : i16_fromPosOverflow;          
        }
      }

      sig32 <<= shiftDist;
      //get the integer 15bits and signal bit
      sig16 = sig32 & 0x7FFF;
      return sign ? -sig16 : sig16;
      
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

  
  return softfloat_roundToI16( 
    sign, (uint_fast32_t) sig32, roundingMode, exact );

}