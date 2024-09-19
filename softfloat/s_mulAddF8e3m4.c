
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

// TODO this func only copy from f8e4m3 
float8_e3m4_t softfloat_mulAddF8e3m4(
     uint_fast8_t uiA, uint_fast8_t uiB, uint_fast8_t uiC, uint_fast8_t op )
{
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA;
    bool signB;
    int_fast8_t expB;
    uint_fast8_t sigB;
    bool signC;
    int_fast8_t expC;
    uint_fast8_t sigC;
    bool signProd;
    uint_fast8_t magBits, uiZ;
    struct exp8_sig8 normExpSig;
    int_fast8_t expProd;
    uint_fast32_t sigProd;
    bool signZ;
    int_fast8_t expZ;
    uint_fast8_t sigZ;
    int_fast8_t expDiff;
    uint_fast32_t sig32Z, sig32C;
    int_fast8_t shiftDist;
    union ui8_f8e3m4 uZ;
    uint_fast8_t numRoundingBits;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    signA = signF8E3M4UI( uiA );
    expA  = expF8E3M4UI( uiA );
    sigA  = fracF8E3M4UI( uiA );
    signB = signF8E3M4UI( uiB );
    expB  = expF8E3M4UI( uiB );
    sigB  = fracF8E3M4UI( uiB );
    signC = signF8E3M4UI( uiC ) ^ (op == softfloat_mulAdd_subC);
    expC  = expF8E3M4UI( uiC );
    sigC  = fracF8E3M4UI( uiC );
    signProd = signA ^ signB ^ (op == softfloat_mulAdd_subProd);
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0x7 ) {
    if ( (sigA == 0xF) || ((expB == 0x7) && (sigB == 0xF)) ) goto propagateNaN;
    if ( (sigA == 0x6) && (expB >= 0x7) && (signProd == signC) ) goto clamp;
    }
    if ( expB == 0xF ) {
        if ( sigB == 0x7 ) goto propagateNaN;
        if ( (sigB == 0x6) && (expA >= 0x7) && (signProd == signC) ) goto clamp;
    }
    if ( expC == 0xF ) {
        if (sigC == 0x7) goto propagateNaN;
        if( (sigC == 0x6) && (signProd == signC) ) goto clamp;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expA ) {
        if ( ! sigA ) goto zeroProd;
        normExpSig = softfloat_normSubnormalF8e3m4Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    if ( ! expB ) {
        if ( ! sigB ) goto zeroProd;
        normExpSig = softfloat_normSubnormalF8e3m4Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expProd = expA + expB - 0x6;
    sigA = (sigA | 0x08)<<3;
    sigB = (sigB | 0x08)<<3;
    sigProd = (uint_fast32_t) sigA * sigB << 16;
    if ( sigProd < 0x20000000 ) {
        --expProd;
        sigProd <<= 1;
    }
    signZ = signProd;
    if ( ! expC ) {
        if ( ! sigC ) {
            expZ = expProd - 1;
            sig32Z = sigProd << 1;
            goto roundPack;
        }
        normExpSig = softfloat_normSubnormalF8e3m4Sig( sigC );
        expC = normExpSig.exp;
        sigC = normExpSig.sig;
    }
    sig32C = (sigC | 0x08) << 26;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expDiff = expProd - expC;
    if ( signProd == signC ) {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        if ( expDiff < 0 ) {
            expZ = expC;
            sig32Z = sig32C + softfloat_shiftRightJam32( sigProd, -expDiff );
        } else if (! expDiff){
            expZ = expC;
            sig32Z = sig32C + sigProd;
        } else {
            expZ = expProd;
            sig32Z = sigProd + softfloat_shiftRightJam32( sig32C, expDiff );
        }

        if ( sig32Z < 0x40000000 ) {
            --expZ;
            sig32Z <<= 1;
        }
    } else {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        if ( expDiff < 0 ) {
            signZ = signC;
            expZ = expC;
            sig32Z = sig32C - softfloat_shiftRightJam32( sigProd, -expDiff );
        } else if ( ! expDiff ) {
            expZ = expProd;
            sig32Z = sigProd - sig32C;
            if ( ! sig32Z ) goto completeCancellation;
            if ( sig32Z & 0x80000000 ) {
                signZ = ! signZ;
                sig32Z = -sig32Z;
            }
        } else {
            expZ = expProd;
            sig32Z = sigProd - softfloat_shiftRightJam32( sig32C, expDiff );
        }
        shiftDist = softfloat_countLeadingZeros32( sig32Z ) - 1;
        expZ -= shiftDist;
        sig32Z = sig32Z << shiftDist;

    }
 roundPack:
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
    sigZ = (sig32Z>>24) | ((sig32Z & 0xFFFFFF) != 0 );
    return softfloat_roundPackToF8e3m4( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 clamp:
    if( (expC == 0xF) && (sigC == 0x7) ) goto propagateNaN;
    uiZ = packToF8E3M4UI( signProd, 0x7, 0 );
    goto uiZ;
 propagateNaN:
    uiZ = defaultNaNF8e3m4UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zeroProd:
    uiZ = uiC;
    if ( ! (expC | sigC) && (signProd != signC) ) {
 completeCancellation:
        uiZ =
            packToF8E3M4UI(
                (softfloat_roundingMode == softfloat_round_min), 0, 0 );
    }
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

