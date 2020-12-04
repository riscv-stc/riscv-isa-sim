
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "softfloat.h"


float16_t i8_to_f16( int8_t a )
{
    bool sign;
    uint_fast32_t absA;
    int_fast8_t shiftDist;
    union ui16_f16 u;
    uint_fast16_t sig;

    //signal 
    sign = (a < 0);

    //abs value
    absA = sign ? -(uint_fast8_t) a : (uint_fast8_t) a;
    
    //shift distance = leaingzeros - (int bits - (1 + frac bits))
    //move the effective bits to frac bits
    shiftDist = softfloat_countLeadingZeros8_( absA ) + 3;
    
    //if 1+ frac bits >= int effective bits
    if ( 0 <= shiftDist ) {
        u.ui =
            a ? packToF16UI(// 0x18 - shiftDist = 15(bias) - shiftDist(absA move) + 9(Frac MSB)
                    sign, 0x18 - shiftDist, (uint_fast16_t) ((absA & 0xFF)<<shiftDist) )
                : 0;
        return u.f;
    } else {
        //if 1+frac bits < int effective bits ,
        //softfloat_roundPackToF16 need left move 4bits
        //to reserve 4 bit to handle the decimal bits
        shiftDist += 4;
        sig =
            (shiftDist < 0)
                ? absA >> (-shiftDist)
                      | ((uint32_t) (absA<<(shiftDist & 31)) != 0)
                : (uint_fast16_t) absA << shiftDist;
        //0x1C-shiftDist = 15(bias) -shiftDist(absA move) + 9(Frac MSB) + 4(shift more 4) 
        return softfloat_roundPackToF16( sign, 0x1C - shiftDist, sig );
    }

}