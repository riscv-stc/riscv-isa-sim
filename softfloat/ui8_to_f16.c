
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "softfloat.h"


float16_t ui8_to_f16( uint8_t a )
{
    int_fast8_t shiftDist;
    union ui16_f16 u;
    uint_fast16_t sig;

    //shift distance = leaingzeros - (uint bits - (1 + frac bits))
    //move the effective bits to frac bits
    shiftDist = softfloat_countLeadingZeros8_( a ) + 3;

    //if 1+ frac bits >= int effective bits
    if ( 0 <= shiftDist ) {
        u.ui =
            a ? packToF16UI(// 0x18 - shiftDist = 0x15(bias) - shiftDist(absA move) + 9(Frac MSB)
                    0, 0x18 - shiftDist, (uint_fast16_t) a<<shiftDist )
                : 0;
        return u.f;
    } else {
        //if 1+frac bits < int effective bits ,
        //softfloat_roundPackToF16 need left move 4bits
        //to reserve 4 bit to handle the decimal bits
        shiftDist += 4;
        sig =
            (shiftDist < 0)
                ? a >> (-shiftDist)
                      | ((uint32_t) (a<<(shiftDist & 31)) != 0)
                : (uint_fast16_t) a << shiftDist;
        //0x1C-shiftDist = 15(bias) -shiftDist(absA move) + 9(Frac MSB) + 4(shift more 4) 
        return softfloat_roundPackToF16( 0, 0x1C - shiftDist, sig );
    }

}