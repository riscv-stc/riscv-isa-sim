#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float32_t f32_reciprocal( float32_t a )
{
  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast32_t sigA;

  uint_fast32_t uiZ;
  union ui32_f32 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF32UI( uiA );
  expA = expF32UI( uiA );
  sigA = fracF32UI( uiA );

  //if expA = 0xff, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
      uZ.ui = uiZ;
      return uZ.f;
    }

    //infinite number input
    uiZ = signA ? 0x80000000 : 0x0;// -0 or 0
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if 0 or little number 
  if( ( a.v & 0x7FFFFFFF ) <= 0x200000 )
  {
    uiZ = signA ? 0xff800000 : 0x7f800000;//-inf or inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  /*------------------------------------------------------------------------
  use the rsqrt value's square to compute the reciprocal value
  *------------------------------------------------------------------------*/
  //get the absolute value of a
  uZ.ui = uiA & 0x7FFFFFFF;

  //calculate the rsqrt value of a
  uZ.f = f32_rsqrt( uZ.f );

  //calculate the reciprocal value 
  uZ.f = f32_mul( uZ.f, uZ.f );

  //recover the signal bit
  uZ.ui = uZ.ui | (signA << 31);

  return uZ.f;

} 