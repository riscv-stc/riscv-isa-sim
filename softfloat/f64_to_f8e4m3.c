
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

float8_e4m3_t f64_to_f8e4m3( float64_t a )
{
    union ui64_f64 uA;
    uint_fast64_t uiA;
    bool sign;
    int_fast16_t exp;
    uint_fast64_t frac;
    struct commonNaN commonNaN;
    uint_fast8_t uiZ, frac8;
    union ui8_f8e4m3 uZ;
    uint_fast8_t numRoundingBits;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    sign = signF64UI( uiA );
    exp  = expF64UI( uiA );
    frac = fracF64UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( exp == 0x7FF ) {
        if ( frac ) {
            softfloat_f64UIToCommonNaN( uiA, &commonNaN );
            uiZ = softfloat_commonNaNToF8e4m3UI( &commonNaN );
        } else {
            uiZ = packToF8E4M3UI( sign, 0xF, 0x7 );
        }
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
   if ( ! (exp | frac) ) {
        uiZ = packToF8E4M3UI( sign, 0, 0 );
        goto uiZ;
    }

    if(softfloat_stochasticRoundingFlag){
        frac = (frac << 10) | 0x4000000000000000;
        
        if( exp < 1013 ){
            uiZ = packToF8E4M3UI( sign, 0, 0 );
            goto uiZ;
        }else if( exp < 1017 ) {
            numRoundingBits = 1076 - exp;
            frac = softfloat_stochasticRound64(frac, numRoundingBits);
            // printf("frac = 0x%lx***", frac);
        }else {
            frac = softfloat_stochasticRound64(frac, 59);
        }
        frac8 = frac >> 0x38;
    } else{
        frac8 = frac >> 0x2E | ((frac & 0x3FFFFFFFFFFF) != 0) | 0x40;
    }
    
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    return softfloat_roundPackToF8e4m3( sign, exp - 0x3F9, frac8 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

