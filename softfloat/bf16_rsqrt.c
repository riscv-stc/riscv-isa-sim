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
  int_fast16_t expA;
  uint_fast8_t sigA;

  uint_fast16_t uiZ;
  union ui16_bf16 uZ;

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
    //if -0
    if( ( 0 == expA ) && (0 == sigA )  )
    {//return -inf
      uiZ = 0xff80;
      uZ.ui = uiZ;
      return uZ.f;
    }

    //if other negative numbers, invalid 
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = 0xffc0;  //-NaN
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if expA = 0xff, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = 0x7fc0; //NaN
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
    uiZ = 0x7f80;
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
  bfloat16_t makeup9 = { 0x480 }; 
  if( 0 == expA )
  {//subnormal  number * 2^8 -> exp = 1 + 8, frac -> 1 + frac, so need subtract 1*2^(1+8)
    a.v += ( 9 << 7 );
    a = bf16_sub( a, makeup9 );
    uiA = a.v;
  }

  uZ.ui = 0x5f38 - ( uiA >> 1 );

  //Iterative calculation
  u_three_half.ui = 0x3FC0;
  //a*0.5
  a.v -= ( 1 << 7 );
  if( 0 == ( a.v & 0x7F80 ) )
  {//subnormal number
    a.v = ( ( a.v & 0x7F ) >> 1 ) + ( a.v & 0x8000 );
    a.v += ( 1 << 6 );
  }

  uZ.f = bf16_mul( uZ.f, bf16_sub( u_three_half.f, bf16_mul( bf16_mul( a, uZ.f ), uZ.f ) ) );

  if( 0 == expA )
  {//subnormal number * 2^4
    uZ.ui += ( 4 << 7 );
  }
  return uZ.f;
  
} 