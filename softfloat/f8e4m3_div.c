
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

extern const uint8_t softfloat_approxRecip_8b[];

float8_e4m3_t f8e4m3_div( float8_e4m3_t a, float8_e4m3_t b )
{
    union ui8_f8e4m3 uA;
    uint_fast8_t uiA;
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA;
    union ui8_f8e4m3 uB;
    uint_fast8_t uiB;
    bool signB;
    int_fast8_t expB;
    uint_fast8_t sigB;
    bool signZ;
    struct exp8_sig8 normExpSig;
    int_fast8_t expZ;
#ifdef SOFTFLOAT_FAST_DIV16TO8
    uint_fast16_t sig16A;
    uint_fast8_t sigZ;
#else
    int index;
    uint8_t r0;
    uint_fast8_t sigZ;
    uint_fast8_t rem;
#endif
    uint_fast8_t uiZ;
    union ui8_f8e4m3 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF8E4M3UI( uiA );
    expA  = expF8E4M3UI( uiA );
    sigA  = fracF8E4M3UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signF8E4M3UI( uiB );
    expB  = expF8E4M3UI( uiB );
    sigB  = fracF8E4M3UI( uiB );
    signZ = signA ^ signB;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( (expB == 0xF) && (sigB == 0x7) ) goto propagateNaN;
    if ( expA == 0xF ) {
        if (sigA == 0x7) goto propagateNaN;
        if ( (sigA == 0x6) && (expB < 0x7) ) goto clamp;
        }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expB ) {
        if ( ! sigB ) {
            if ( ! (expA | sigA) ) goto invalid;
            softfloat_raiseFlags( softfloat_flag_infinite );
            goto clamp;
        }
        normExpSig = softfloat_normSubnormalF8e4m3Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    if ( ! expA ) {
        if ( ! sigA ) goto zero;
        normExpSig = softfloat_normSubnormalF8e4m3Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = expA - expB + 0x6;
    sigA |= 0x8;
    sigB |= 0x8;
#ifdef SOFTFLOAT_FAST_DIV16TO8
    if ( sigA < sigB ) {
        --expZ;
        sig16A = (uint_fast16_t) sigA<<7;
    } else {
        sig16A = (uint_fast16_t) sigA<<6;
    }
    sigZ = sig16A / sigB;
    if ( ! (sigZ & 3) ) sigZ |= ((uint_fast16_t) sigB * sigZ != sig16A);
#else
    if ( sigA < sigB ) {
        --expZ;
        sigA <<= 4;
    } else {
        sigA <<= 3;
    }
    index = sigB & 0x7;
    r0 = softfloat_approxRecip_8b[index];
    sigZ = ((uint_fast16_t) sigA * r0)>>8;

    rem = (sigA<<3) - sigZ * sigB;
    sigZ += (rem * (uint_fast16_t) r0)>>11;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    ++sigZ;
    if ( ! (sigZ & 3) ) {
        sigZ &= ~1;
        rem = (sigA<<3) - sigZ * sigB;
        if ( rem & 0x80 ) {
            sigZ -= 1;
        }
    }
#endif
    return softfloat_roundPackToF8e4m3( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    // uiZ = softfloat_propagateNaNF16UI( uiA, uiB );
    uiZ = defaultNaNF8e4m3UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNF8e4m3UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 clamp:
    uiZ = packToF8E4M3UI( signZ, 0xF, 0x7 );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uiZ = packToF8E4M3UI( signZ, 0, 0 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

