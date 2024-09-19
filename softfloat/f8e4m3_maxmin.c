
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3d, by John R. Hauser.

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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"


float8_e4m3_t f8e4m3_max( float8_e4m3_t a, float8_e4m3_t b )          
{                                                                                              
    bool greater = f8e4m3_lt_quiet(b, a) ||                                             
               (f8e4m3_eq(b, a) && signF8E4M3UI(b.v));                           

    if (isNaNF8E4M3UI(a.v) && isNaNF8E4M3UI(b.v)) {                              
        union ui8_f8e4m3  ui;                                                    
        ui.ui = defaultNaNF8e4m3UI;                                                     
        return ui.f;                                                                           
    } else {                                                                                   
        return greater || isNaNF8E4M3UI((b).v) ? a : b;                                 
    }                                                                                          
}


float8_e4m3_t f8e4m3_min( float8_e4m3_t a, float8_e4m3_t b )          
{                                                                                              
    bool less = f8e4m3_lt_quiet(a, b) ||                                                
               (f8e4m3_eq(a, b) && signF8E4M3UI(a.v));                           
                                                                                               
    if (isNaNF8E4M3UI(a.v) && isNaNF8E4M3UI(b.v)) {                              
        union ui8_f8e4m3  ui;                                                    
        ui.ui = defaultNaNF8e4m3UI;                                                     
        return ui.f;                                                                           
    } else {                                                                                   
        return less || isNaNF8E4M3UI((b).v) ? a : b;                                    
    }                                                                                          
}

// COMPARE_MAX(a, b, 16);

