
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

float8_e4m3_t softfloat_addMagsF8e4m3( uint_fast8_t uiA, uint_fast8_t uiB )
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
    expA = expF8E4M3UI( uiA );
    sigA = fracF8E4M3UI( uiA );
    expB = expF8E4M3UI( uiB );
    sigB = fracF8E4M3UI( uiB );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expDiff = expA - expB;
    if ( ! expDiff ) { 
        signZ = signF8E4M3UI( uiA );
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        if ( ! expA ) {     
            uiZ = uiA + sigB;
            goto uiZ;
        }
        if ( expA == 0xF ) {
            if ((sigA == 0x7) || (sigB == 0x7)) goto propagateNaN;
            goto clampMaxMag;
        }
        
        expZ = expA;
        sigZ = 0x10 + sigA + sigB;
        if ( ! (sigZ & 1) && (expZ < 0xe) ) { //last digit of sigZ is zero 
            sigZ >>= 1;
            goto pack;
        }
        sigZ <<= 2;
    } else {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        signZ = signF8E4M3UI( uiA );
        if ( expDiff < 0 ) {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
           if ( expB == 0xf ) {
                if ( sigB == 0x7 ) goto propagateNaN;
                if (sigB == 0x6) goto clampMaxMag;
            }

            if ( expDiff <= -5 ) {
                uiZ = packToF8E4M3UI( signZ, expB, sigB );
                if(!(expA | sigA)) goto uiZ;
                if( (!softfloat_stochasticRoundingFlag) || (expDiff < -12) ) { 
                    goto addEpsilon;
                }
            }
            expZ = expB;
            sigX = sigB | 0x8;  //Implicit mantissa bit in exp
            sigY = sigA + (expA ? 0x8 : sigA);
            shiftDist = 26 + expDiff;   
        } else {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            uiZ = uiA;
            if ( expA == 0xf ) {
                if ( sigA == 0x7 ) goto propagateNaN;
                if (sigA == 0x6) goto clampMaxMag;
            }
            if ( 5 <= expDiff ) {
                if(!(expB | sigB)) goto uiZ;
                if( (!softfloat_stochasticRoundingFlag) || (expDiff > 12) ) { 
                    goto addEpsilon;
                }
            }
            expZ = expA;
            sigX = sigA | 0x8;
            sigY = sigB + (expB ? 0x8 : sigB);
            shiftDist = 26 - expDiff;
        }
        sig32Z =
            ((uint_fast16_t) sigX<<26) + ((uint_fast16_t) sigY<<shiftDist);
        if ( sig32Z < 0x40000000 ) {
            --expZ;
            sig32Z <<= 1;
        }
        if(softfloat_stochasticRoundingFlag){
            if( expZ <= -4 ){
                uiZ = packToF8E4M3UI( signZ, 0, 0 );
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
            if ( ! (sigZ & 0x7) && (expZ < 0xE) ) {
                sigZ >>= 3;
                goto pack;
            }
        }
    }
    return softfloat_roundPackToF8e4m3( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
clampMaxMag:
    uiZ = packToF8E4M3UI( signZ, 0XF, 0X6 );
    goto uiZ;
   /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = defaultNaNF8e4m3UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 addEpsilon:
    roundingMode = softfloat_roundingMode;
    if ( roundingMode != softfloat_round_near_even ) {
        if (
            roundingMode
                == (signF8E4M3UI( uiZ ) ? softfloat_round_min
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
    uiZ = packToF8E4M3UI( signZ, expZ, sigZ );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

