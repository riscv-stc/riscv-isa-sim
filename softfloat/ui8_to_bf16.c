
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "softfloat.h"


bfloat16_t ui8_to_bf16( uint8_t a )
{
    int_fast8_t shiftDist;
    union ui16_bf16 u;
    uint_fast16_t sig;


    //shift distance = leadingzeros - (uint8 bits - (1 + frac bits))
    //move the effective bits to frac bits
    shiftDist = softfloat_countLeadingZeros8_( a );

    //if 1+ frac bits >= int effective bits
    if ( 0 <= shiftDist ) {
        u.ui =
            a ? packToBF16UI(// 0x85 - shiftDist = 127(bias) - shiftDist(absA move) + 6(Frac MSB)
                    0, 0x85 - shiftDist, (uint_fast16_t) a<<shiftDist )
                : 0;
        return u.f;
    } else {
        //softfloat_roundPackToBF16 need left move 4bit
        shiftDist += 4;
        sig =
            (shiftDist < 0)
                ? a >> (-shiftDist)
                      | ((uint32_t) (a<<(shiftDist & 31)) != 0)
                : (uint_fast16_t) a << shiftDist;
        //0x89-shiftDist = 0x7F(bias) -shiftDist(absA move) + 6(Frac MSB) + 4(shift more 4) 
        return softfloat_roundPackToBF16( 0, 0x89 - shiftDist, sig );
    }

}