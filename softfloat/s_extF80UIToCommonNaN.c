
/*----------------------------------------------------------------------------
| This file intentionally contains no code.
*----------------------------------------------------------------------------*/

// #include <stdint.h>
// #include "platform.h"
// #include "specialize.h"
// #include "softfloat.h"

// /*----------------------------------------------------------------------------
// | Assuming the unsigned integer formed from concatenating `uiA64' and `uiA0'
// | has the bit pattern of an 80-bit extended floating-point NaN, converts
// | this NaN to the common NaN form, and stores the resulting common NaN at the
// | location pointed to by `zPtr'.  If the NaN is a signaling NaN, the invalid
// | exception is raised.
// *----------------------------------------------------------------------------*/
// void
//  softfloat_extF80UIToCommonNaN(
//      uint_fast16_t uiA64, uint_fast64_t uiA0, struct commonNaN *zPtr )
// {

//     if ( softfloat_isSigNaNExtF80UI( uiA64, uiA0 ) ) {
//         softfloat_raiseFlags( softfloat_flag_invalid );
//     }
//     zPtr->sign = uiA64>>15;
//     zPtr->v64  = uiA0<<1;
//     zPtr->v0   = 0;

// }

