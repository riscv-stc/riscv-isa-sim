#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float32_t f32_reciprocal( float32_t a )
{
  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast32_t sigA;

  struct exp16_sig32 normExpSig;

  union ui32_f32 uT;

  uint_fast32_t uiZ;
  int_fast8_t expZ;
  uint_fast32_t sigZ;
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
      uiZ = softfloat_propagateNaNF32UI( uiA, 0 );
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0x80000000 : 0x0;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if 0 or subnormal number 
  if( 0 == expA )
  {
    //if 0, invalid 
    if( 0 == sigA )
    {
      softfloat_raiseFlags( softfloat_flag_invalid );
      uiZ = defaultNaNF32UI;
      uZ.ui = uiZ;
      return uZ.f;
    }
    //handle subnormal number
    normExpSig = softfloat_normSubnormalF32Sig( sigA ); 
    expA = normExpSig.exp;
    sigA = normExpSig.sig;
    uA.ui = packToF32UI( signA, expA, sigA);
    uiA = uA.ui;
    a = uA.f;
  }

  /*------------------------------------------------------------------------
  use the rsqrt value's square to compute the reciprocal value
  *------------------------------------------------------------------------*/
  //get the absolute value of a
  uT.ui = uiA & 0x7FFFFFFF;

  //calculate the rsqrt value of a
  uT.f = f32_rsqrt( uT.f );

  //calculate the reciprocal value 
  uZ.f = f32_mul( uT.f, uT.f );

  //recover the signal bit
  uZ.ui = uZ.ui | (signA << 31);

  return uZ.f;

} 