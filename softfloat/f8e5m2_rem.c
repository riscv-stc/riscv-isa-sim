
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

float8_e5m2_t f8e5m2_rem( float8_e5m2_t a, float8_e5m2_t b )
{
    union ui8_f8e5m2 uA;
    uint_fast8_t uiA;
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA;
    union ui8_f8e5m2 uB;
    uint_fast8_t uiB;
    int_fast8_t expB;
    uint_fast8_t sigB;
    struct exp8_sig8 normExpSig;
    uint8_t rem;
    int_fast8_t expDiff;
    uint_fast8_t q;
    uint32_t recip32, q32;
    uint8_t altRem, meanRem;
    bool signRem;
    uint_fast8_t uiZ;
    union ui8_f8e5m2 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF8E5M2UI( uiA );
    expA  = expF8E5M2UI( uiA );
    sigA  = fracF8E5M2UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    expB = expF8E5M2UI( uiB );
    sigB = fracF8E5M2UI( uiB );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0x1F ) {
        if ( sigA || ((expB == 0x1F) && sigB) ) goto propagateNaN;
        goto invalid;
    }
    if ( expB == 0x1F ) {
        if ( sigB ) goto propagateNaN;
        return a;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expB ) {
        if ( ! sigB ) goto invalid;
        normExpSig = softfloat_normSubnormalF8e5m2Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    if ( ! expA ) {
        if ( ! sigA ) return a;
        normExpSig = softfloat_normSubnormalF8e5m2Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    rem = sigA | 0x04;
    sigB |= 0x04;
    expDiff = expA - expB;
    if ( expDiff < 1 ) {
        if ( expDiff < -1 ) return a;
        sigB <<= 3;
        if ( expDiff ) {
            rem <<= 2;
            q = 0;
        } else {
            rem <<= 3;
            q = (sigB <= rem);
            if ( q ) rem -= sigB;
        }
    } else {
        recip32 = softfloat_approxRecip32_1( (uint_fast32_t) sigB<<29 );
        /*--------------------------------------------------------------------
        | Changing the shift of `rem' here requires also changing the initial
        | subtraction from `expDiff'.
        *--------------------------------------------------------------------*/
        rem <<= 4;
        expDiff -= 31;
        /*--------------------------------------------------------------------
        | The scale of `sigB' affects how many bits are obtained during each
        | cycle of the loop.  Currently this is 29 bits per loop iteration,
        | which is believed to be the maximum possible.
        *--------------------------------------------------------------------*/
        sigB <<= 3;
        for (;;) {
            q32 = (rem * (uint_fast64_t) recip32)>>8;
            if ( expDiff < 0 ) break;
            rem = -((uint_fast8_t) q32 * sigB);
            expDiff -= 29;
        }
        /*--------------------------------------------------------------------
        | (`expDiff' cannot be less than -30 here.)
        *--------------------------------------------------------------------*/
        q32 >>= ~expDiff & 31;
        q = q32;
        rem = (rem<<(expDiff + 30)) - q * sigB;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    do {
        altRem = rem;
        ++q;
        rem -= sigB;
    } while ( ! (rem & 0x80) );
    meanRem = rem + altRem;
    if ( (meanRem & 0x80) || (! meanRem && (q & 1)) ) rem = altRem;
    signRem = signA;
    if ( 0x80 <= rem ) {
        signRem = ! signRem;
        rem = -rem;
    }
    return softfloat_normRoundPackToF8e5m2( signRem, expB, rem );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = softfloat_propagateNaNF8e5m2UI( uiA, uiB );
    goto uiZ;
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNF8e5m2UI;
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

