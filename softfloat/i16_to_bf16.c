
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "softfloat.h"

bfloat16_t i16_to_bf16( int16_t a )
{
    bool sign;
    uint_fast32_t absA;
    int_fast8_t shiftDist;
    union ui16_bf16 u;
    uint_fast16_t sig;

    //signal 
    sign = (a < 0);

    //abs value
    absA = sign ? -(uint_fast16_t) a : (uint_fast16_t) a;

    //shift distance = leadingzeros - (int16 bits - (1 + frac bits))
    //move the effective bits to frac bits
    shiftDist = softfloat_countLeadingZeros16( absA ) - 8;

    //if 1+ frac bits >= int effective bits
    if ( 0 <= shiftDist ) {
        u.ui =
            a ? packToBF16UI(// 0x85 - shiftDist = 127(bias) - shiftDist(absA move) + 6(Frac MSB)
                    sign, 0x85 - shiftDist, (uint_fast16_t) ( absA & 0xFFFF) << shiftDist )
                : 0;
        return u.f;
    } else {
        //softfloat_roundPackToBF16 need left move 4bit
        shiftDist += 4;
        sig =
            (shiftDist < 0)
                ? absA >> (-shiftDist)
                      | ((uint32_t) (absA<<(shiftDist & 31)) != 0)
                : (uint_fast16_t) absA << shiftDist;
        //0x89-shiftDist = 0x7F(bias) -shiftDist(absA move) + 6(Frac MSB) + 4(shift more 4) 
        return softfloat_roundPackToBF16( sign, 0x89 - shiftDist, sig );
    }

}