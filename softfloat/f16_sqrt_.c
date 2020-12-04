#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float16_t f16_sqrt_( float16_t a )
{
  union ui16_f16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast16_t sigA;

  struct exp8_sig16 normExpSig;

  float16_t t;

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
    uiZ = defaultNaNF16UI;
    uZ.ui = uiZ;
    return uZ.f;
  }

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
    //positive infinite number input
    return a;
  }

  //if 0 or positive subnormal number 
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
    normExpSig = softfloat_normSubnormalF16Sig( sigA ); 
    expA = normExpSig.exp;
    sigA = normExpSig.sig;
    uA.ui = packToF16UI( 0, expA, sigA);
    uiA = uA.ui;
    a = uA.f;
  }

  /*------------------------------------------------------------------------
  use the product of a and rsqrt of a to compute the sqrt value of a
  *------------------------------------------------------------------------*/

  //compute the rsqrt value of a
  t = f16_rsqrt( a );

  //return the product of a and rsqrt value
  return f16_mul( a, t );

} 