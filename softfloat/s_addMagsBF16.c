
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

bfloat16_t softfloat_addMagsBF16( uint_fast16_t uiA, uint_fast16_t uiB )
{
    int_fast16_t expA;
    uint_fast16_t sigA;
    int_fast16_t expB;
    uint_fast16_t sigB;
    int_fast16_t expDiff;
    uint_fast16_t uiZ;
    bool signZ;
    int_fast16_t expZ;
    uint_fast16_t sigZ;
    uint_fast16_t sigX, sigY;
    int_fast16_t shiftDist;
    uint_fast32_t sig32Z;
    int_fast8_t roundingMode;
    union ui16_bf16 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expA = expBF16UI( uiA );
    sigA = fracBF16UI( uiA );
    expB = expBF16UI( uiB );
    sigB = fracBF16UI( uiB );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expDiff = expA - expB;
    if ( ! expDiff ) {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        if ( ! expA ) {
            uiZ = uiA + sigB;
            goto uiZ;
        }
        if ( expA == 0xFF ) {
            if ( sigA | sigB ) goto propagateNaN;
            uiZ = uiA;
            goto uiZ;
        }
        signZ = signBF16UI( uiA );
        expZ = expA;
        // 0x100: 第8位是隐藏位，两个隐藏位相加就是0x100
        sigZ = 0x0100 + sigA + sigB;
        // 0xFE:0x100已经计入exp了，如果这里条件设置0xFF，会发生阶码上溢出
        if ( ! (sigZ & 1) && (expZ < 0xFE) ) {
            sigZ >>= 1;
            goto pack;
        }
        sigZ <<= 3;
    } else {
        /*--------------------------------------------------------------------
        *--------------------------------------------------------------------*/
        signZ = signBF16UI( uiA );
        if ( expDiff < 0 ) {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            if ( expB == 0xFF ) {
                if ( sigB ) goto propagateNaN;
                uiZ = packToBF16UI( signZ, 0xFF, 0 );
                goto uiZ;
            }
            if ( expDiff <= -10 ) {
                uiZ = packToBF16UI( signZ, expB, sigB );
                if ( expA | sigA ) goto addEpsilon;
                goto uiZ;
            }
            expZ = expB;
            sigX = sigB | 0x0080;
            sigY = sigA + (expA ? 0x0080 : sigA);
            shiftDist = 22 + expDiff;
        } else {
            /*----------------------------------------------------------------
            *----------------------------------------------------------------*/
            uiZ = uiA;
            if ( expA == 0xFF ) {
                if ( sigA ) goto propagateNaN;
                goto uiZ;
            }
            // 7 + (22 - 10) - 19 = 0
            // 后面有两个移位，左移 22 - expDiff， 右移16，这里的10就是个临界点，小数位是否会被全部移空的临界点
            if ( 10 <= expDiff ) {
                if ( expB | sigB ) goto addEpsilon;
                goto uiZ;
            }
            expZ = expA;
            sigX = sigA | 0x0080;
            sigY = sigB + (expB ? 0x0080 : sigB);
            // 22： 将尾数扩展到30位。 因为低阶向高阶对阶码，位数会右移动，影响精确度。为了保护尾数，将A，B都左移扩展到30位。
            // 这个22是不是有点大了？fp16移动了19，bfp16是不是16就足够了
            shiftDist = 22 - expDiff;
        }
        sig32Z =
            ((uint_fast32_t) sigX<<22) + ((uint_fast32_t) sigY<<shiftDist);
        if ( sig32Z < 0x40000000 ) {
            --expZ;
            sig32Z <<= 1;
        }
        sigZ = sig32Z>>19;
        // 右边移掉的19位是否要舍“入”
        if ( sig32Z & 0x7FFFF ) {
            sigZ |= 1;
        } else {
            if ( ! (sigZ & 0xF) && (expZ < 0xFE) ) {
                sigZ >>= 4;
                goto pack;
            }
        }
    }
    return softfloat_roundPackToBF16( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = softfloat_propagateNaNBF16UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 // 进到这里代表有一个浮点数极小, uiZ此时等于另一个数，这里只是要考虑是否要将这个极小数舍入uiZ
 addEpsilon:
    roundingMode = softfloat_roundingMode;
    if ( roundingMode != softfloat_round_near_even ) {
        if (
            roundingMode
                == (signBF16UI( uiZ ) ? softfloat_round_min
                        : softfloat_round_max)
        ) {
            ++uiZ;
            // 确认uiZ的加1后是否变成无穷大
            if ( (uint16_t) (uiZ<<1) == 0xFF00 ) {
                softfloat_raiseFlags(
                    softfloat_flag_overflow | softfloat_flag_inexact );
            }
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
    uiZ = packToBF16UI( signZ, expZ, sigZ );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

