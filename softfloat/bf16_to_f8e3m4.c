
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015 The Regents of the University of
California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

float8_e3m4_t bf16_to_f8e3m4( bfloat16_t a )
{
    union ui16_bf16 uA;
    uint_fast16_t uiA, frac16;
    bool sign;
    int_fast16_t exp;
    uint_fast8_t frac;
    struct commonNaN commonNaN;
    uint_fast8_t uiZ, frac8;
    union ui8_f8e3m4 uZ;
    uint_fast8_t numRoundingBits;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    sign = signBF16UI( uiA );
    exp  = expBF16UI( uiA );
    frac = fracBF16UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( exp == 0xFF ) {
        if ( frac ) {
            softfloat_bf16UIToCommonNaN( uiA, &commonNaN );
            uiZ = softfloat_commonNaNToF8e3m4UI( &commonNaN );
        } else {
            uiZ = packToF8E3M4UI( sign, 0x7, 0xF );
        }
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
   if ( ! (exp | frac) ) {
            uiZ = packToF8E3M4UI( sign, 0, 0 );
            goto uiZ;
        }
   if(softfloat_stochasticRoundingFlag){
        frac16 = (frac << 7) | 0x4000;
        // 121 = (127 - 3) + 1 - 5 , 127 is bf16 deviation value, 3 is f8e3m4 deviation value, 5 = 4(frac) + 1(implict bit)
        if( exp < 120 ){
            uiZ = packToF8E3M4UI( sign, 0, 0 );
            goto uiZ;
        }else if( exp < 125 ) { // 126 = (127 - 3) + 1
            // bf16 data is 15 bit ,so 136 = 121 + 15
            numRoundingBits = 135 - exp;
            frac16 = softfloat_stochasticRound16(frac16, numRoundingBits);
        }else {
            // bf16 frac is 7bit , f8e3m4 exp is 3bit 10 = 7 + 3
            frac16 = softfloat_stochasticRound16(frac16, 10);
        }
        frac8 = frac16 >> 8;
    } else{
        frac8 = frac >> 1 | ((frac & 0x1) != 0) | 0x40;
    }
    
    
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    // 7D = 127 - 3 + 1
    return softfloat_roundPackToF8e3m4( sign, exp - 0x7D, frac8 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

