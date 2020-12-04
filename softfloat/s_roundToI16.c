#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

/*
 * the low 12bits of sig are decimal bits
 */
int_fast16_t
 softfloat_roundToI16(
     bool sign, uint_fast64_t sig, uint_fast8_t roundingMode, bool exact )
{
    uint_fast16_t roundIncrement, roundBits;
    uint_fast32_t sig32;
    uint_fast16_t sig16;
    union { uint16_t ui; int16_t i; } uZ;
    int_fast16_t z;

    /*
      * calculate the roundIncrement bits
      */
    //frac bits + 2  the decimal  MSB is No 11 bit (0.5)
    roundIncrement = 0x800;
    if (
        (roundingMode != softfloat_round_near_maxMag)
            && (roundingMode != softfloat_round_near_even)
    ) 
    {
      roundIncrement = 0;// no decimal bits
      if ( 
          sign
              ? (roundingMode == softfloat_round_min)
#ifdef SOFTFLOAT_ROUND_ODD
                    || (roundingMode == softfloat_round_odd)
#endif
              : (roundingMode == softfloat_round_max)
      ) 
      {
          roundIncrement = 0xFFF; // all decimal bits are 1
      }

    }

    //get the decimal bits
    roundBits = sig & 0xFFF;

    //round the number
    sig += roundIncrement;

    //overflow handle 16(int16) + 12bit(decimal)
    if ( sig & UINT64_C( 0xFFFFFFFFF0000000 ) ) goto invalid;

    //get the integer bits
    sig32 = sig >> 12;

    //if decimal bits is 0.5 and round to even, clear the LSB 1
    if (
        (roundBits == 0x800) && (roundingMode == softfloat_round_near_even)
    ) 
    {
        sig32 &= ~(uint_fast32_t) 1;
    }

    //get the integer 16bits and signal bit
    sig16 = sig32 & 0xFFFF;
    uZ.ui = sign ? -sig16 : sig16;
    z = uZ.i;

    //-0
    if ( z && ((z < 0) ^ sign) ) goto invalid;

    //exact flag
    if ( roundBits ) 
    {
#ifdef SOFTFLOAT_ROUND_ODD
        if ( roundingMode == softfloat_round_odd ) z |= 1;
#endif
        if ( exact ) 
        {
          softfloat_exceptionFlags |= softfloat_flag_inexact;
        }
    }

    return z;

    /*
     * invalid handle
     */
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    return sign ? i16_fromNegOverflow : i16_fromPosOverflow;

}