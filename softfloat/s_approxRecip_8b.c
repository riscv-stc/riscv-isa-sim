
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

#include <stdint.h>
#include "platform.h"
#include "primitives.h"

const uint8_t softfloat_approxRecip_8b[8] = {
   //  0xFF, 0xE4, 0xCD, 0xBA, 0xAB, 0x9E, 0x92, 0x88
   0xFF, 0xE3, 0xCC, 0xBA, 0xAA, 0x9D, 0x92, 0x88
};

const uint8_t softfloat_approxRecip_8b_0s[16] = {
   //  0xFF, 0xE4, 0xCD, 0xBA, 0xAB, 0x9E, 0x92, 0x88
   0xFF, 0xF0, 0xE3, 0xD7, 0xCC, 0xC2, 0xBA, 0xB2,
    0xAA, 0xA3, 0x9D, 0x97, 0x92, 0x8D, 0x88, 0x84
};

const uint8_t softfloat_approxRecip_8b_8s[16] = {
   //  0xFF, 0xE4, 0xCD, 0xBA, 0xAB, 0x9E, 0x92, 0x88
   0xF0, 0xD6, 0xBF, 0xAC, 0x9C, 0x8D, 0x81, 0x76,
    0x6D, 0x64, 0x5D, 0x56, 0x50, 0x4B, 0x46, 0x42
};

const uint8_t softfloat_approxRecip_8b_1s[8] = {
   //  0xFF, 0xE4, 0xCD, 0xBA, 0xAB, 0x9E, 0x92, 0x88
   0xF0, 0xBF, 0x9C, 0x81,
    0x6D, 0x5D, 0x50, 0x46
};

