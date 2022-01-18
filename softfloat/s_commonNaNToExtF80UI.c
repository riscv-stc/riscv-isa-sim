
/*----------------------------------------------------------------------------
| This file intentionally contains no code.
*----------------------------------------------------------------------------*/

// #include <stdint.h>
// #include "platform.h"
// #include "primitives.h"
// #include "specialize.h"

// /*----------------------------------------------------------------------------
// | Converts the common NaN pointed to by `aPtr' into an 80-bit extended
// | floating-point NaN, and returns the bit pattern of this value as an unsigned
// | integer.
// *----------------------------------------------------------------------------*/
// struct uint128 softfloat_commonNaNToExtF80UI( const struct commonNaN *aPtr )
// {
//     struct uint128 uiZ;

//     uiZ.v64 = (uint_fast16_t) aPtr->sign<<15 | 0x7FFF;
//     uiZ.v0 = UINT64_C( 0xC000000000000000 ) | aPtr->v64>>1;
//     return uiZ;

// }

