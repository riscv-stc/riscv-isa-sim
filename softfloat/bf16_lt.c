
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015 The Regents of the University of
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
#include "softfloat.h"

bool bf16_lt( bfloat16_t a, bfloat16_t b )
{
   union ui16_bf16 uA;
   uint_fast16_t uiA;
   union ui16_bf16 uB;
   uint_fast16_t uiB;
   bool signA, signB;

   //get the bits of bfloat16 number
   uA.f = a;
   uiA = uA.ui;
   uB.f = b;
   uiB = uB.ui;

   //if a or b is NaN, return false
   if ( isNaNBF16UI( uiA ) || isNaNBF16UI( uiB ) ) 
   {
      softfloat_raiseFlags( softfloat_flag_invalid );
      return false;
   }

   //get the signal bit of a and b
   signA = signBF16UI( uiA );
   signB = signBF16UI( uiB );
   /**
    * if signA == signB:
    * if a == b, return false(so (uiA != uiB) && ), 
    * else if sign == 0, uiA < uiB return true,  sign == 1 uiA < uiB return false, so use signA ^ ( uiA < uiB ) to do that.
    * if signA != signB, if signA == 0, return false
    * when signA == 1, then if uiA = 0x8000(-0) and uiB == 0x0000(0) then return false, else return true
    */
   return
      (signA != signB) ? signA && ((uint16_t) ((uiA | uiB)<<1) != 0)
      : (uiA != uiB) && (signA ^ (uiA < uiB));

    return true;

}

