#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include <stdio.h>

bfloat16_t bf16_tanh( bfloat16_t a )
{

  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast16_t sigA;

  uint_fast16_t uiZ;
  union ui16_bf16 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signBF16UI( uiA );
  expA = expBF16UI( uiA );
  sigA = fracBF16UI( uiA );

  //if expA = 0xff, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xffc0 : 0x7fc0;//-NaN or NaN
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0xbf80 : 0x3f80;//-1 or 1
    uZ.ui = uiZ;
    return uZ.f;
  }

  if ( ( 0xc05d < a.v ) )// a < -3.453125
  {
    uZ.ui = 0xbf80;//-1
    return uZ.f;
  }
  if ( ( 0x405d < a.v ) && ( 0x8000 > a.v ))//a>3.453125
  {
    uZ.ui = 0x3f80;//1
    return uZ.f;
  }

  /** when x=nln2, the tanh(x) and all levels of deratives are easy to compute, 
   * so use the taylor series when x close to x0=nln2.
   */ 
  bfloat16_t ln2_val, ln2_recip;
  ln2_val.v = 0x3f31; // ln2=0.6931471805599453
  ln2_recip.v = 0x3fb9; // 1/ln2=1.4426950408889634

  bfloat16_t n = bf16_roundToInt( bf16_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2) -14~14
  int16_t n_int = bf16_to_i16( n, softfloat_round_near_maxMag, false );
  bfloat16_t x0 = bf16_mul( n, ln2_val ); // x0 = n * ln2
  bfloat16_t dx = bf16_sub( a, x0 );

  //taylor series
  bfloat16_t val_n[] = { 0xbf80, 0xbf7e, 0xbf78, 0xbf62, 0xbf1a, 0x0000, 0x3f1a, 0x3f62, 0x3f78, 0x3f7e, 0x3f80 };
  bfloat16_t der_1[] = { 0x3b80, 0x3c7e, 0x3d78, 0x3e63, 0x3f24, 0x3f80, 0x3f24, 0x3e63, 0x3d78, 0x3c7e, 0x3b80 };
  bfloat16_t der_2[] = { 0x3b7f, 0x3c7c, 0x3d71, 0x3e48, 0x3ec5, 0x0000, 0xbec5, 0xbe48, 0xbd71, 0xbc7c, 0xbb7f };
  bfloat16_t der_3[] = { 0x3b29, 0x3c25, 0x3d16, 0x3dca, 0x3c8c, 0xbeab, 0x3c8c, 0x3dca, 0x3d16, 0x3c25, 0x3b29 };

  uZ.f = bf16_add( val_n[ n_int + 5 ], bf16_mul( dx,
        bf16_add( der_1[ n_int + 5 ], bf16_mul( dx,
        bf16_add( der_2[ n_int + 5 ], bf16_mul( dx, 
        der_3[ n_int + 5 ] ) ) ) ) ) );
  return uZ.f;
  
}
