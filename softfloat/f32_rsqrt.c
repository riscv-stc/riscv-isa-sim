#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float32_t f32_rsqrt( float32_t a )
{
  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast32_t sigA;
  uint_fast32_t uiZ;
  union ui32_f32 uZ;

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
    //if -0
    if( ( 0 == expA ) && (0 == sigA )  )
    {//return -inf
      uiZ = 0xff800000;
      uZ.ui = uiZ;
      return uZ.f;
    }

    //if other negative numbers, invalid 
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = 0xffc00000;  //-NaN
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if expA = 0xff, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = 0x7fc00000; //NaN
      uZ.ui = uiZ;
      return uZ.f;
    }
    //positive infinite number input, return 0
    uiZ = 0;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if 0 
  if( ( 0 == expA ) && ( 0 == sigA ) )
  {
    //return +inf
    uiZ = 0x7f800000;
    uZ.ui = uiZ;
    return uZ.f;    
    
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

  float32_t makeup25 = { 0x0c800000 }; 
  if( 0 == expA )
  {//subnormal  number * 2^24 -> exp = 1 + 24, frac -> 1 + frac, so need subtract 1*2^(1+24)
    a.v += ( 25 << 23 );
    a = f32_sub( a, makeup25 );
    uiA = a.v;
  }

  uZ.ui = 0x5f3759df - ( uiA >> 1 );

  //Iterative calculation
  u_three_half.ui = 0x3FC00000;
  //a*0.5
  a.v -= ( 1 << 23 );
  if( 0 == ( a.v & 0x7F800000 ) )
  {//subnormal number
    a.v = ( ( a.v & 0x7FFFFF ) >> 1 ) + ( a.v & 0x80000000 );
    a.v += ( 1 << 22 );
  }

  uZ.f = f32_mul( uZ.f, f32_sub( u_three_half.f, f32_mul( f32_mul( a, uZ.f ), uZ.f ) ) );

  if( 0 == expA )
  {//subnormal number * 2^12
    uZ.ui += ( 12 << 23 );
  }
  return uZ.f;
} 