#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float32_t f32_rsqrt( float32_t a )
{
  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast32_t sigA;

  struct exp16_sig32 normExpSig;

  uint_fast32_t uiZ;
  int_fast8_t expZ;
  uint_fast32_t sigZ;
  union ui32_f32 uZ;

  union ui32_f32 u_half;
  union ui32_f32 u_three_half;
  float32_t a_half;



  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF32UI( uiA );
  expA = expF32UI( uiA );
  sigA = fracF32UI( uiA );

  //if negative number
  if( 1 == signA )
  {
    //if -0 or other negative numbers, invalid 
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
      uiZ = defaultNaNF32UI;
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
  uZ.ui = 0x5f3759df - ( uiA >> 1 );

  //Iterative calculation
  u_half.ui = 0x3F000000;
  u_three_half.ui = 0x3FC00000;
  a_half = f32_mul( a, u_half.f );
  uZ.f = f32_mul( uZ.f, f32_sub( u_three_half.f, f32_mul( f32_mul( a_half, uZ.f ), uZ.f ) ) );

  return uZ.f;

} 