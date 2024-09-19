
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016 The Regents of the University of
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

extern const uint8_t softfloat_approxRecipSqrt_8b[];

float8_e4m3_t f8e4m3_sqrt( float8_e4m3_t a )
{
    union ui8_f8e4m3 uA;
    uint_fast8_t uiA;
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA, uiZ;
    struct exp8_sig8 normExpSig;
    int_fast8_t expZ;
    int index;
    // uint_fast16_t r0;
    // uint_fast32_t ESqrR0;
    // uint16_t sigma0;
    uint_fast8_t recipSqrt8;
    uint_fast8_t sigZ, shiftedSigZ;

    uint8_t negRem;
    union ui8_f8e4m3 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF8E4M3UI( uiA );
    expA  = expF8E4M3UI( uiA );
    sigA  = fracF8E4M3UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0xF && sigA == 0x7) {
            uiZ = defaultNaNF8e4m3UI;
            goto uiZ;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( signA ) {
        if ( ! (expA | sigA) ) return a;
        goto invalid;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expA ) {
        if ( ! sigA ) return a;
        normExpSig = softfloat_normSubnormalF8e4m3Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = ((expA - 0x7)>>1) + 0x6;
    expA &= 1;
    sigA |= 0x8;
    index = (sigA<<1 & 0xE) + expA;
    recipSqrt8 = softfloat_approxRecipSqrt_8b[index];
    sigZ = ((uint_fast16_t) (sigA<<4) * recipSqrt8)>>8;
  
    if ( expA ) sigZ  >>= 1;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    ++sigZ;
    if ( ! (sigZ & 3) ) {
        shiftedSigZ = sigZ>>1;
        negRem = shiftedSigZ * shiftedSigZ;
        if ( negRem & 0x40 ) {
            sigZ |= 1;
        } else {
            if ( negRem ) --sigZ;
        }
    }
    return softfloat_roundPackToF8e4m3( 0, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNF8e4m3UI;
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

