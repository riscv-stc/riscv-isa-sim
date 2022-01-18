
/*----------------------------------------------------------------------------
| This file intentionally contains no code.
*----------------------------------------------------------------------------*/

// #include <stdint.h>
// #include "platform.h"
// #include "primitives.h"
// #include "specialize.h"
// #include "softfloat.h"

// /*----------------------------------------------------------------------------
// | Assuming the unsigned integer formed from concatenating `uiA64' and `uiA0'
// | has the bit pattern of a 128-bit floating-point NaN, converts this NaN to
// | the common NaN form, and stores the resulting common NaN at the location
// | pointed to by `zPtr'.  If the NaN is a signaling NaN, the invalid exception
// | is raised.
// *----------------------------------------------------------------------------*/
// void
//  softfloat_f128UIToCommonNaN(
//      uint_fast64_t uiA64, uint_fast64_t uiA0, struct commonNaN *zPtr )
// {
//     struct uint128 NaNSig;

//     if ( softfloat_isSigNaNF128UI( uiA64, uiA0 ) ) {
//         softfloat_raiseFlags( softfloat_flag_invalid );
//     }
//     NaNSig = softfloat_shortShiftLeft128( uiA64, uiA0, 16 );
//     zPtr->sign = uiA64>>63;
//     zPtr->v64  = NaNSig.v64;
//     zPtr->v0   = NaNSig.v0;

// }

