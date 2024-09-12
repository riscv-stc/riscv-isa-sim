
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

float8_e3m4_t softfloat_addMagsF8e3m4( uint_fast8_t uiA, uint_fast8_t uiB )
{
    int_fast8_t expA;
    uint_fast8_t sigA;
    int_fast8_t expB;
    uint_fast8_t sigB;
    int_fast8_t expDiff;
    uint_fast8_t uiZ;
    bool signZ;
    int_fast8_t expZ;
    uint_fast8_t sigZ;
    uint_fast8_t sigX, sigY;
    int_fast8_t shiftDist;
    uint_fast32_t sig32Z;
    int_fast8_t roundingMode;
    union ui8_f8e4m3 uZ;
    uint_fast8_t numRoundingBits;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expA = expF8E3M4UI( uiA );
    sigA = fracF8E3M4UI( uiA );
    expB = expF8E3M4UI( uiB );
    sigB = fracF8E3M4UI( uiB );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expDiff = expA - expB;
    if ( ! expDiff ) { 
        signZ = signF8E3M4UI( uiA );
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        if ( ! expA ) {     
            uiZ = uiA + sigB;
            goto uiZ;
        }
        if ( expA == 0x7 ) {
            if ((sigA == 0xF) || (sigB == 0xF)) goto propagateNaN;
            goto clampMaxMag;
        }
        
        expZ = expA;
        sigZ = 0x20 + sigA + sigB;
        if ( ! (sigZ & 1) && (expZ < 0x6) ) { //last digit of sigZ is zero 
            sigZ >>= 1;
            goto pack;
        }
        sigZ <<= 1;
    } else {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        signZ = signF8E3M4UI( uiA );
        if ( expDiff < 0 ) {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
           if ( expB == 0x7 ) {
                if ( sigB == 0xF ) goto propagateNaN;
                if (sigB == 0x0) goto clampMaxMag;
            }
            // 2 − 2^(w−1) ≤ e ≤ 2^(w−1) − 1, w = 3 , -2 <= e <= 3, 1 <= exp <= 6
            // -2 = 1 - 3, exp min is 1 , exp 0 is 3
            if ( expDiff <= -2 ) {
                uiZ = packToF8E3M4UI( signZ, expB, sigB );
                if(!(expA | sigA)) goto uiZ;
                if( (!softfloat_stochasticRoundingFlag) ) { 
                    goto addEpsilon;
                }
            }
            expZ = expB;
            sigX = sigB | 0x10;  //Implicit mantissa bit in exp
            sigY = sigA + (expA ? 0x10 : sigA);
            shiftDist = 30 + expDiff;   
        } else {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            uiZ = uiA;
            if ( expA == 0x7 ) {
                if ( sigA == 0xF ) goto propagateNaN;
                if (sigA == 0xE) goto clampMaxMag;
            }
            if ( 2 <= expDiff ) {
                if(!(expB | sigB)) goto uiZ;
                if( (!softfloat_stochasticRoundingFlag) ) { 
                    goto addEpsilon;
                }
            }
            expZ = expA;
            sigX = sigA | 0x10;
            sigY = sigB + (expB ? 0x10 : sigB);
            shiftDist = 30 - expDiff;
        }
        sig32Z =
            ((uint_fast16_t) sigX<<30) + ((uint_fast16_t) sigY<<shiftDist);
        if ( sig32Z < 0x40000000 ) {
            --expZ;
            sig32Z <<= 1;
        }
        if(softfloat_stochasticRoundingFlag){
            if( expZ <= -4 ){
                uiZ = packToF8E3M4UI( signZ, 0, 0 );
                goto uiZ;
            }else if( expZ < 0 ) {
                numRoundingBits = 27 - expZ;
                sig32Z = softfloat_stochasticRound32(sig32Z, numRoundingBits);
            }else {
                sig32Z = softfloat_stochasticRound32(sig32Z, 27);
            }
        }
        sigZ = sig32Z>> 24 ;
        if ( sig32Z & 0xFFFFFF ) {
            sigZ |= 1;
        } else {
            if ( ! (sigZ & 0xF) && (expZ < 0x6) ) {
                sigZ >>= 4;
                goto pack;
            }
        }
    }
    return softfloat_roundPackToF8e3m4( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
clampMaxMag:
    uiZ = packToF8E3M4UI( signZ, 0XF, 0X0 );
    goto uiZ;
   /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = defaultNaNF8e3m4UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 addEpsilon:
    roundingMode = softfloat_roundingMode;
    if ( roundingMode != softfloat_round_near_even ) {
        if (
            roundingMode
                == (signF8E3M4UI( uiZ ) ? softfloat_round_min
                        : softfloat_round_max)
        ) {
            ++uiZ;
        }
#ifdef SOFTFLOAT_ROUND_ODD
        else if ( roundingMode == softfloat_round_odd ) {
            uiZ |= 1;
        }
#endif
    }
    softfloat_exceptionFlags |= softfloat_flag_inexact;
        goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 pack:
    uiZ = packToF8E3M4UI( signZ, expZ, sigZ );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

