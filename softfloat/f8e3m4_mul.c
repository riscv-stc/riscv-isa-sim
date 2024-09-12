
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

// TODO only copy from f8e4m3
float8_e3m4_t f8e3m4_mul( float8_e3m4_t a, float8_e3m4_t b )
{
    union ui8_f8e3m4 uA;
    uint_fast8_t uiA;
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA;
    union ui8_f8e3m4 uB;
    uint_fast8_t uiB;
    bool signB;
    int_fast8_t expB;
    uint_fast8_t sigB;
    bool signZ;
    uint_fast8_t magBits;
    struct exp8_sig8 normExpSig;
    int_fast8_t expZ;
    uint_fast16_t sig16Z;
    uint_fast8_t sigZ, uiZ;
    union ui8_f8e3m4 uZ;
    uint_fast8_t numRoundingBits;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF8E3M4UI( uiA );
    expA  = expF8E3M4UI( uiA );
    sigA  = fracF8E3M4UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signF8E3M4UI( uiB );
    expB  = expF8E3M4UI( uiB );
    sigB  = fracF8E3M4UI( uiB );
    signZ = signA ^ signB;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    // if ( (expA == 0xf && sigA == 0x7) || (expB == 0xf && sigB == 0x7) ) goto propagateNaN;
    // if ( (expA == 0xF) && (sigA == 0x6) && (expB >= 0x7)) goto clamp;
    // if ( (expB == 0xF) && (sigB == 0x6) && (expA >= 0x7)) goto clamp;
    // }
///////////////////////////////////////////////////////////////////////////////////////    17
    if ( expA == 0x7 ) {
    if ( (sigA == 0xF) || ((expB == 0x7) && (sigB == 0xF)) ) goto propagateNaN;
    if ( (sigA == 0xE) && (expB >= 0x3)) goto clamp;
    }
    if ( expB == 0x7 ) {
        if ( sigB == 0xF ) goto propagateNaN;
        if ( (sigB == 0xE) && (expA >= 0x3)) goto clamp;
    }
 
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expA ) {
        if ( ! sigA ) goto zero;
        normExpSig = softfloat_normSubnormalF8e3m4Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    if ( ! expB ) {
        if ( ! sigB ) goto zero;
        normExpSig = softfloat_normSubnormalF8e3m4Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = expA + expB - 0x3;
    sigA = (sigA | 0x10)<<3;
    sigB = (sigB | 0x10)<<4;
    sig16Z = (uint_fast16_t) sigA * sigB;
    if ( sig16Z < 0x4000 ) {
        --expZ;
        sig16Z <<= 1;
    }
    if(softfloat_stochasticRoundingFlag){
            if( expZ <= -4 ){
                uiZ = packToF8E3M4UI( signZ, 0, 0 );
                goto uiZ;
            }else if( expZ < 0 ) {
                numRoundingBits = 11 - expZ;
                sig16Z = softfloat_stochasticRound16(sig16Z, numRoundingBits);
            }else {
                sig16Z = softfloat_stochasticRound16(sig16Z, 11);
            }
        }
    sigZ = sig16Z>>8;
    if ( sig16Z & 0xFF ) sigZ |= 1;
    
    return softfloat_roundPackToF8e3m4( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = defaultNaNF8e3m4UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
clamp:
    uiZ = packToF8E3M4UI( signZ, 0x7, 0xE );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uiZ = packToF8E3M4UI( signZ, 0, 0 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

