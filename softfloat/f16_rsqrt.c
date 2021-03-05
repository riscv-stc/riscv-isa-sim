#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

float16_t f16_rsqrt( float16_t a )
{
  union ui16_f16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast16_t sigA;

  uint_fast16_t uiZ;
  union ui16_f16 uZ;

  union ui16_f16 u_three_half;
  float16_t a_half;



  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF16UI( uiA );
  expA = expF16UI( uiA );
  sigA = fracF16UI( uiA );

  //if negative number
  if( 1 == signA )
  {
    //if -0
    if( ( 0 == expA ) && (0 == sigA )  )
    {//return -inf
      uiZ = 0xfc00;
      uZ.ui = uiZ;
      return uZ.f;
    }

    //if other negative numbers, invalid 
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = 0xfe00;  //-NaN
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if expA = 0x1f, handle the NaN and infinite number
  if( 0x1F == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = 0x7e00; //NaN
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
    uiZ = 0x7c00;
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

  float16_t makeup11 = { 0x2c00 }; 
  if( 0 == expA )
  {//subnormal  number * 2^20 -> exp = 1 + 10, frac -> 1 + frac, so need subtract 1*2^(1+10) 
    a.v += ( 11 << 10 );
    a = f16_sub( a, makeup11 );
    uiA = a.v;
  }

  uZ.ui = 0x59ba - ( uiA >> 1 );  
  

  //Iterative calculation
  u_three_half.ui = 0x3E00;
  //a*0.5
  a.v -= ( 1 << 10 );
  if( 0 == ( a.v & 0x7c00 ) )
  {//subnormal number
    a.v = ( ( a.v & 0x3FF ) >> 1 ) + ( a.v & 0x8000 );
    a.v += ( 1 << 9 );
  }

  uZ.f = f16_mul( uZ.f, f16_sub( u_three_half.f, f16_mul( f16_mul( a, uZ.f ), uZ.f ) ) );

  //makeup table, the index is the exp bits
  uint16_t makeup[ ] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
  uint8_t index = a.v >> 10;
  uZ.ui += makeup[ index ];

  if( 0 == expA )
  {//subnormal number * 2^5
    uZ.ui += ( 5 << 10 );
  }

  return uZ.f;

} 