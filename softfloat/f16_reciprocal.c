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

  struct exp8_sig16 normExpSig;

  union ui16_f16 uT;

  uint_fast16_t uiZ;
  int_fast8_t expZ;
  uint_fast16_t sigZ;
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
      uiZ = softfloat_propagateNaNF16UI( uiA, 0 );
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0x8000 : 0x0;
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
      uiZ = defaultNaNF16UI;
      uZ.ui = uiZ;
      return uZ.f;
    }
    //handle subnormal number
    normExpSig = softfloat_normSubnormalF16Sig( sigA ); 
    expA = normExpSig.exp;
    sigA = normExpSig.sig;
    uA.ui = packToF16UI( signA, expA, sigA);
    uiA = uA.ui;
    a = uA.f;    

  }

  /*------------------------------------------------------------------------
  use the rsqrt value's square to compute the reciprocal value
  *------------------------------------------------------------------------*/
  //get the absolute value of a
  uT.ui = uiA & 0x7FFF;

  //calculate the rsqrt value of a
  uT.f = f16_rsqrt( uT.f );

  //calculate the reciprocal value 
  uZ.f = f16_mul( uT.f, uT.f );

  //recover the signal bit
  uZ.ui = uZ.ui | (signA << 15);

  return uZ.f;

} 