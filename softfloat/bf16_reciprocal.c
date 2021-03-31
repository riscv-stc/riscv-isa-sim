#include "platform.h"
#include "softfloat_types.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

bfloat16_t bf16_reciprocal( bfloat16_t a )
{
  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast8_t sigA;

  union ui16_bf16 uT;

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
    uiZ = signA ? 0x8000 : 0x0;// -0 or 0
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if 0 or little number 
  if( ( a.v & 0x7FFF ) <= 0x20 )
  {
    uiZ = signA ? 0xff80 : 0x7f80;//-inf or inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  /*------------------------------------------------------------------------
  use the rsqrt value's square to compute the reciprocal value
  *------------------------------------------------------------------------*/
  //get the absolute value of a
  uT.ui = uiA & 0x7FFF;

  //calculate the rsqrt value of a
  uT.f = bf16_rsqrt( uT.f );

  //calculate the reciprocal value 
  uZ.f = bf16_mul( uT.f, uT.f );

  //recover the signal bit
  uZ.ui = uZ.ui | (signA << 15);


  return uZ.f;
  
} 