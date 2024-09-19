
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2017 The Regents of the
University of California.  All rights reserved.

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
uint_fast8_t softfloat_countLeadingZeros16( uint16_t a );

float8_e5m2_t softfloat_subMagsF8e5m2( uint_fast8_t uiA, uint_fast8_t uiB )
{
    int_fast8_t expA;
    uint_fast8_t sigA;
    int_fast8_t expB;
    uint_fast8_t sigB;
    int_fast8_t expDiff;
    uint_fast8_t uiZ;
    int_fast8_t sigDiff;
    bool signZ;
    int_fast8_t shiftDist, expZ;
    uint_fast8_t sigZ, sigX, sigY;
    uint_fast16_t sig32Z;
    int_fast8_t roundingMode;
    union ui8_f8e5m2 uZ;
    uint_fast8_t numRoundingBits;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expA = expF8E5M2UI( uiA );
    sigA = fracF8E5M2UI( uiA );
    expB = expF8E5M2UI( uiB );
    sigB = fracF8E5M2UI( uiB );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expDiff = expA - expB;
    if ( ! expDiff ) {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        if ( expA == 0x1F ) {
            if ( sigA | sigB ) goto propagateNaN;
            softfloat_raiseFlags( softfloat_flag_invalid );
            uiZ = defaultNaNF8e5m2UI;
            goto uiZ;
        }
        sigDiff = sigA - sigB;
        if ( ! sigDiff ) {
            uiZ =
                packToF8E5M2UI(
                    (softfloat_roundingMode == softfloat_round_min), 0, 0 );
            goto uiZ;
        }
        if ( expA ) --expA;
        signZ = signF8E5M2UI( uiA );
        if ( sigDiff < 0 ) {
            signZ = ! signZ;
            sigDiff = -sigDiff;
        }
        shiftDist = softfloat_countLeadingZeros8_( sigDiff ) - 5;
        expZ = expA - shiftDist;
        if ( expZ < 0 ) {
            shiftDist = expA;
            expZ = 0;
        }
        sigZ = sigDiff<<shiftDist;
        goto pack;
    } else {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        signZ = signF8E5M2UI( uiA );
        if ( expDiff < 0 ) {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            signZ = ! signZ;
            if ( expB == 0x1F ) {
                if ( sigB ) goto propagateNaN;
                uiZ = packToF8E5M2UI( signZ, 0x1F, 0 );
                goto uiZ;
            }
            if ( expDiff <= -5 ) {
                uiZ = packToF8E5M2UI( signZ, expB, sigB );
                if(!(expA | sigA)) goto uiZ;
                if( (!softfloat_stochasticRoundingFlag) || (expDiff < -28) ) {
                    goto subEpsilon;
                }
            }
            expZ = expA + 27;
            sigX = sigB | 0x04;
            sigY = sigA + (expA ? 0x04 : sigA);
            expDiff = -expDiff;
        } else {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            uiZ = uiA;
            if ( expA == 0x1F ) {
                if ( sigA ) goto propagateNaN;
                goto uiZ;
            }
            if ( 5 <= expDiff ) {
                if(!(expB | sigB)) goto uiZ;
                if( (!softfloat_stochasticRoundingFlag) || (expDiff > 28) ) {
                    goto subEpsilon;
                }
            }
            expZ = expB + 27;
            sigX = sigA | 0x04;
            sigY = sigB + (expB ? 0x04 : sigB);
        }
        sig32Z = ((uint_fast32_t) sigX<<expDiff) - sigY;
        shiftDist = softfloat_countLeadingZeros32( sig32Z ) - 1;
        sig32Z <<= shiftDist;
        expZ -= shiftDist;
        if(softfloat_stochasticRoundingFlag){
            if( expZ <= -3 ){
                uiZ = packToF8E5M2UI( signZ, 0, 0 );
                goto uiZ;
            }else if( expZ < 0 ) {
                numRoundingBits = 28 - expZ;
                sig32Z = softfloat_stochasticRound32(sig32Z, numRoundingBits);
            }else {
                sig32Z = softfloat_stochasticRound32(sig32Z, 28);
            }
        }

        sigZ = sig32Z >> 24;
        if ( sig32Z & 0xFFFFFF ) {
            sigZ |= 1;
        } else {
            if ( ! (sigZ & 0xF) && ((unsigned int) expZ < 0x1E) ) {
                sigZ >>= 4;
                goto pack;
            }
        }
        return softfloat_roundPackToF8e5m2( signZ, expZ, sigZ );
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = softfloat_propagateNaNF8e5m2UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 subEpsilon:
    roundingMode = softfloat_roundingMode;
    if ( roundingMode != softfloat_round_near_even ) {
        if (
            (roundingMode == softfloat_round_minMag)
                || (roundingMode
                        == (signF8E5M2UI( uiZ ) ? softfloat_round_max
                                : softfloat_round_min))
        ) {
            --uiZ;
        }
#ifdef SOFTFLOAT_ROUND_ODD
        else if ( roundingMode == softfloat_round_odd ) {
            uiZ = (uiZ - 1) | 1;
        }
#endif
    }
    softfloat_exceptionFlags |= softfloat_flag_inexact;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 pack:
    uiZ = packToF8E5M2UI( signZ, expZ, sigZ );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

