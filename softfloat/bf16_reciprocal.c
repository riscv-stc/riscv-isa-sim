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
  int_fast8_t expA;
  uint_fast8_t sigA;

  struct exp8_sig16 normExpSig;

  union ui16_bf16 uT;

  uint_fast16_t uiZ;
  int_fast8_t expZ;
  uint_fast8_t sigZ;
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
      uiZ = softfloat_propagateNaNBF16UI( uiA, 0 );
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
      uiZ = defaultNaNBF16UI;
      uZ.ui = uiZ;
      return uZ.f;
    }
    //handle subnormal number
    normExpSig = softfloat_normSubnormalBF16Sig( sigA ); 
    expA = normExpSig.exp;
    sigA = normExpSig.sig;
    uA.ui = packToBF16UI( signA, expA, sigA);
    uiA = uA.ui;
    a = uA.f;

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