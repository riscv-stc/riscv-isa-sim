#include "platform.h"
#include "softfloat_types.h"
#include "softfloat.h"
#include "specialize.h"
#include "internals.h"

bfloat16_t bf16_sqrt_( bfloat16_t a )
{
  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast8_t sigA;

  struct exp8_sig16 normExpSig;

  bfloat16_t t;

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
    uiZ = defaultNaNBF16UI;
    uZ.ui = uiZ;
    return uZ.f;
  }

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
    normExpSig = softfloat_normSubnormalBF16Sig( sigA ); 
    expA = normExpSig.exp;
    sigA = normExpSig.sig;
    uA.ui = packToBF16UI( 0, expA, sigA);
    uiA = uA.ui;
    a = uA.f;

  }

  /*------------------------------------------------------------------------
  use the product of a and rsqrt of a to compute the sqrt value of a
  *------------------------------------------------------------------------*/

  //compute the rsqrt value of a
  t = bf16_rsqrt( a );

  //return the product of a and rsqrt value
  return bf16_mul( a, t );
  
} 