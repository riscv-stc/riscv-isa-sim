
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

/*
 * the low 12bits of sig are decimal bits
 */
uint_fast8_t
 softfloat_roundToUI8(
     bool sign, uint_fast64_t sig, uint_fast8_t roundingMode, bool exact )
{
    uint_fast16_t roundIncrement, roundBits;
    uint_fast32_t sig32;
    uint_fast8_t sig8;
    

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
      if ( sign ) {
        //negative number 
        
        if ( !sig ) return 0; //-0

        if ( roundingMode == softfloat_round_min ) goto invalid;
#ifdef SOFTFLOAT_ROUND_ODD
        if ( roundingMode == softfloat_round_odd ) goto invalid;
#endif
      } 
      else 
      {
        //positive number
        if ( roundingMode == softfloat_round_max ) 
        {
          roundIncrement = 0xFFF; // all decimal bits are 1
        }
      }

    }

    //get the decimal bits
    roundBits = sig & 0xFFF;

    //round the number
    sig += roundIncrement;

    //overflow handle 8(int8) + 12bit(decimal)
    if ( sig & UINT64_C( 0xFFFFFFFFFFF00000 ) ) goto invalid;

    //get the integer bits
    sig32 = sig >> 12;

    //if decimal bits is 0.5 and round to even, clear the LSB 1
    if (
        (roundBits == 0x800) && (roundingMode == softfloat_round_near_even)
    ) 
    {
        sig32 &= ~(uint_fast32_t) 1;
    }

    //get the integer 8bits
    sig8 = sig32 & 0xFF;

    //negative number 
    if ( sign && sig8 ) goto invalid;

    //exact flag
    if ( roundBits ) 
    {
#ifdef SOFTFLOAT_ROUND_ODD
        if ( roundingMode == softfloat_round_odd ) sig8 |= 1;
#endif
        if ( exact ) 
        {
          softfloat_exceptionFlags |= softfloat_flag_inexact;
        }
    }

    return sig8;

    /*
     * invalid handle
     */
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    return sign ? ui8_fromNegOverflow : ui8_fromPosOverflow;

}