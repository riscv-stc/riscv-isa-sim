#include "platform.h"
#include "softfloat_types.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

bfloat16_t bf16_rsqrt( bfloat16_t a )
{
  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast8_t sigA;

  struct exp8_sig16 normExpSig;

  uint_fast16_t uiZ;
  int_fast8_t expZ;
  uint_fast8_t sigZ;
  union ui16_bf16 uZ;

  union ui16_bf16 u_half;
  union ui16_bf16 u_three_half;
  bfloat16_t a_half;



  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signBF16UI( uiA );
  expA = expBF16UI( uiA );
  sigA = fracBF16UI( uiA );

  //if negative number
  if( 1 == signA )
  {
    //if -0 or other negative numbers, invalid 
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
    uiZ = 0;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if 0 or postiove subnormal number 
  if( 0 == expA )
  {
    //if +0, invalid 
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
    uA.ui = packToBF16UI( 0, expA, sigA);
    uiA = uA.ui;
    a = uA.f;

  }

  /*------------------------------------------------------------------------
  use the Newton-Raphson Method(NR) to calculate the rsqrt value

  The equations are as follows:
  
  function:y=f(x)

  the first derivative value is y'=f'(x)

  so the nth approximate value can be calculated 

    x[n+1] = x[n] - f(x[n]) / f'(x[n])

  In this function, we need to calculate the solution of equation 1/(x^2)-a=0, the NR Iterative formula is 
  
  x[n+1]=1/2*x[n]*(3-a*x[n]*x[n])
  *------------------------------------------------------------------------*/

  //first approximate
  uZ.ui = 0x5f37 - ( uiA >> 1);

  //Iterative calculation
  u_half.ui = 0x3F00;
  u_three_half.ui = 0x3FC0;
  a_half = bf16_mul( a, u_half.f );
  uZ.f = bf16_mul( uZ.f, bf16_sub( u_three_half.f, bf16_mul( bf16_mul( a_half, uZ.f ), uZ.f ) ) );

  return uZ.f;
  
} 