
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

bfloat16_t
 softfloat_mulAddBF16(
     uint_fast16_t uiA, uint_fast16_t uiB, uint_fast16_t uiC, uint_fast8_t op )
{
    bool signA;
    int_fast16_t expA;
    uint_fast16_t sigA;
    bool signB;
    int_fast16_t expB;
    uint_fast16_t sigB;
    bool signC;
    int_fast16_t expC;
    uint_fast16_t sigC;
    bool signProd;
    uint_fast16_t magBits, uiZ;
    struct exp8_sig16 normExpSig;
    int_fast16_t expProd;
    uint_fast32_t sigProd;
    bool signZ;
    int_fast16_t expZ;
    uint_fast16_t sigZ;
    int_fast16_t expDiff;
    uint_fast32_t sig32Z, sig32C;
    int_fast16_t shiftDist;
    union ui16_bf16 uZ;

    /*------------------------------------------------------------------------
      get the signal\exp\fraction bits from input float number
    *------------------------------------------------------------------------*/
    signA = signBF16UI( uiA );
    expA  = expBF16UI( uiA );
    sigA  = fracBF16UI( uiA );
    signB = signBF16UI( uiB );
    expB  = expBF16UI( uiB );
    sigB  = fracBF16UI( uiB );
    signC = signBF16UI( uiC ) ^ (op == softfloat_mulAdd_subC);
    expC  = expBF16UI( uiC );
    sigC  = fracBF16UI( uiC );
    signProd = signA ^ signB ^ (op == softfloat_mulAdd_subProd);
    /*------------------------------------------------------------------------
      if expN = 0xFf, handle the NaN and infinite number
    *------------------------------------------------------------------------*/
    if ( expA == 0xFF ) {
        if ( sigA || ((expB == 0xFF) && sigB) ) goto propagateNaN_ABC;
        magBits = expB | sigB;
        goto infProdArg;
    }
    if ( expB == 0xFF ) {
        if ( sigB ) goto propagateNaN_ABC;
        magBits = expA | sigA;
        goto infProdArg;
    }
    if ( expC == 0xFF ) {
        if ( sigC ) {
            uiZ = 0;
            goto propagateNaN_ZC;
        }
        uiZ = uiC;
        goto uiZ;
    }
    /*------------------------------------------------------------------------
      if expN = 0x0, handle the normal Express formal
    *------------------------------------------------------------------------*/
    if ( ! expA ) {
        if ( ! sigA ) goto zeroProd;
        normExpSig = softfloat_normSubnormalBF16Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    if ( ! expB ) {
        if ( ! sigB ) goto zeroProd;
        normExpSig = softfloat_normSubnormalBF16Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }    
    /*------------------------------------------------------------------------
      handle the mul: adding exponent, multipfying Fraction, handle normal 
      Express formal  
      note: sigProd is 29 bits, sigC is 10 bits
    *------------------------------------------------------------------------*/
    expProd = expA + expB - 0x7E;   
    sigA = (sigA | 0x0080)<<7;  
    sigB = (sigB | 0x0080)<<7;  
    sigProd = (uint_fast32_t) sigA * sigB;
    if ( sigProd < 0x20000000 ) {
        --expProd;
        sigProd <<= 1;
    }
    signZ = signProd;
    if ( ! expC ) {
        if ( ! sigC ) {
            expZ = expProd - 1;
            sigZ = sigProd>>18 | ((sigProd & 0x3FFFF) != 0); //15   0xFFFF
            goto roundPack;
        }
        normExpSig = softfloat_normSubnormalBF16Sig( sigC );
        expC = normExpSig.exp;
        sigC = normExpSig.sig;
    }
    sigC = (sigC | 0x0080)<<3;  //???
    /*------------------------------------------------------------------------
      handle the add: adding according to the bigger exponent  
      note: shiftRight is 19 (29 - 10)
    *------------------------------------------------------------------------*/
    expDiff = expProd - expC;
    if ( signProd == signC ) {
        /*--------------------------------------------------------------------
          same sign: 
        *--------------------------------------------------------------------*/
        if ( expDiff <= 0 ) {  
            expZ = expC;
            sigZ = sigC + softfloat_shiftRightJam32( sigProd, 19 - expDiff );   //16
        } else {                
            expZ = expProd;
            sig32Z =
                sigProd
                    + softfloat_shiftRightJam32(
                          (uint_fast32_t) sigC<<19, expDiff );
            sigZ = sig32Z>>19 | ((sig32Z & 0x3FFFF) != 0 );
        }
        if ( sigZ < 0x0800 ) {  //0x0800=7+4
            --expZ;
            sigZ <<= 1;
        }
    } else {    
        /*--------------------------------------------------------------------
          different sign: sign is according to the bigger exponent 
        *--------------------------------------------------------------------*/
        sig32C = (uint_fast32_t) sigC<<19;  
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
        shiftDist -= 19;    
        if ( shiftDist < 0 ) {
            sigZ =
                sig32Z>>(-shiftDist)
                    | ((uint32_t) (sig32Z<<(shiftDist & 31)) != 0);
        } else {
            sigZ = (uint_fast16_t) sig32Z<<shiftDist;
        }
    }
 roundPack:
    return softfloat_roundPackToBF16( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN_ABC:
    uiZ = softfloat_propagateNaNBF16UI( uiA, uiB );
    goto propagateNaN_ZC;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infProdArg:
    if ( magBits ) {
        uiZ = packToBF16UI( signProd, 0xFF, 0 );
        if ( expC != 0xFF ) goto uiZ;
        if ( sigC ) goto propagateNaN_ZC;
        if ( signProd == signC ) goto uiZ;
    }
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNBF16UI;
 propagateNaN_ZC:
    uiZ = softfloat_propagateNaNBF16UI( uiZ, uiC );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zeroProd:
    uiZ = uiC;
    if ( ! (expC | sigC) && (signProd != signC) ) {
 completeCancellation:
        uiZ =
            packToBF16UI(
                (softfloat_roundingMode == softfloat_round_min), 0, 0 );
    }
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

