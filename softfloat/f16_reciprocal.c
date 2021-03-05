#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float16_t f16_reciprocal( float16_t a )
{
  union ui16_f16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast16_t sigA;

  uint_fast16_t uiZ;
  union ui16_f16 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF16UI( uiA );
  expA = expF16UI( uiA );
  sigA = fracF16UI( uiA );

  //if expA = 0x1f, handle the NaN and infinite number
  if( 0x1F == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xfe00 : 0x7e00;//-NaN or NaN
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0x8000 : 0x0; //-0 or 0
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if 0
  if( ( 0 == expA ) && ( 0 == sigA ) )
  {
      uiZ = signA ? 0xfc00 : 0x7c00; //-inf or inf
      uZ.ui = uiZ;
      return uZ.f; 
  }

  //the approximation result of these two number is 65472, but the exact value is 65536(inf in fp16),so just give the value
  if( 0x100  == ( a.v & 0x7FFF ) )
  {
    uZ.ui  = signA ? 0xfc00 : 0x7c00;
    return uZ.f;
  }

  /*------------------------------------------------------------------------
  use the rsqrt value's square to compute the reciprocal value
  *------------------------------------------------------------------------*/
  //get the absolute value of a
  uZ.ui = uiA & 0x7FFF;

  //calculate the rsqrt value of a
  uZ.f = f16_rsqrt( uZ.f );

  //calculate the reciprocal value 
  uZ.f = f16_mul( uZ.f, uZ.f );

  //recover the signal bit
  uZ.ui = uZ.ui | (signA << 15);

  return uZ.f;

} 