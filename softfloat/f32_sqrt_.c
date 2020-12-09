#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float32_t f32_sqrt_( float32_t a )
{
  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast32_t sigA;

  struct exp16_sig32 normExpSig;

  float32_t t;

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

  //if negative number
  if( 1 == signA )
  {
    if( ! ( expA | sigA ) )
    {
      //if -0
      return a;
    }
    //if other negative numbers, invalid 
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNF32UI;
    uZ.ui = uiZ;
    return uZ.f;
  }

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
    //positive infinite number input
    return a;
  }

  //if 0 or postiove subnormal number 
  if( 0 == expA )
  {
    //if +0, return 0
    if( 0 == sigA )
    {
      uiZ = 0;
      uZ.ui = uiZ;
      return uZ.f;
    }
    //handle subnormal number
    normExpSig = softfloat_normSubnormalF32Sig( sigA ); 
    expA = normExpSig.exp;
    sigA = normExpSig.sig;
    uA.ui = packToF32UI( 0, expA, sigA);
    uiA = uA.ui;
    a = uA.f;
  }

  /*------------------------------------------------------------------------
  use the product of a and rsqrt of a to compute the sqrt value of a
  *------------------------------------------------------------------------*/

  //compute the rsqrt value of a
  t = f32_rsqrt( a );

  //return the product of a and rsqrt value
  return f32_mul( a, t );

} 