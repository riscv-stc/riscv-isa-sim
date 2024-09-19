
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2017 The Regents of the University of
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
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "platform.h"
#include "internals.h"
#include "softfloat.h"

uint16_t softfloat_stochasticRound16( uint16_t sig, uint8_t numRoundingBits ){

    uint16_t mask = (1 << numRoundingBits) - 1;
    uint16_t random_num = rand() & 0xFFFF;
    random_num &= mask;
    sig  = (sig + random_num) & (~mask);
    return sig;

}

uint32_t softfloat_stochasticRound32( uint32_t sig, uint8_t numRoundingBits ){

    uint32_t mask = (1 << numRoundingBits) - 1;
    uint32_t random_num = rand() & 0xFFFFFFFF;
    random_num &= mask;
    sig  = (sig + random_num) & (~mask);
    return sig;

}

uint64_t softfloat_stochasticRound64( uint64_t sig, uint8_t numRoundingBits ){

    uint64_t mask = ((uint64_t)1 << numRoundingBits) - 1;
    // printf("mask = 0x%lx***", mask);
    uint64_t random_num = ((uint64_t)rand() << 32) | (((uint64_t)rand() & 0x7FFFFFFF) << 1);
    // printf("RD_pre = 0x%lx***", random_num);
    random_num &= mask;
    // printf("RD_pst = 0x%lx***", random_num);
    sig  = (sig + random_num) & (~mask);
    return sig;

}