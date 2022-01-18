
/*----------------------------------------------------------------------------
| This file intentionally contains no code.
*----------------------------------------------------------------------------*/

// #include <stdint.h>
// #include "platform.h"
// #include "specialize.h"
// #include "softfloat.h"

// /*----------------------------------------------------------------------------
// | Assuming `uiA' has the bit pattern of a 16-bit floating-point NaN, converts
// | this NaN to the common NaN form, and stores the resulting common NaN at the
// | location pointed to by `zPtr'.  If the NaN is a signaling NaN, the invalid
// | exception is raised.
// *----------------------------------------------------------------------------*/
// void softfloat_bf16UIToCommonNaN( uint_fast16_t uiA, struct commonNaN *zPtr )
// {

//     if ( softfloat_isSigNaNBF16UI( uiA ) ) {
//         softfloat_raiseFlags( softfloat_flag_invalid );
//     }
//     zPtr->sign = uiA>>15;
//     zPtr->v64  = (uint_fast64_t) uiA<<57;
//     zPtr->v0   = 0;

// }

