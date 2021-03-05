#include "platform.h"
#include "internals.h"
#include "softfloat.h"
#include "specialize.h"

uint_fast8_t softfloat_countLeadingZeros64_( uint64_t a )
    { return a ? __builtin_clzll( a ) : 64; }

float16_t f16_sin( float16_t a )
{
  union ui16_f16 uA;

  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast16_t sigA;

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
    uZ.ui = signA ? 0xfe00 : 0x7e00;
    return uZ.f;
  }

  //handle zero and subnormal number, return itself, which is the first item of taylor series
  if( 0 == expA )
  {
    return a;
  }

  //move a to [ -pi/2, pi/2 ]
  if( ( a.v & 0x7FFF ) > 0x3e48 )
  {
  //32bits effective bits for 2pi and recip of 2pi
    int8_t expa, exptwopi, exprecip_twopi;
    uint32_t siga, sigtwopi, sigrecip_twopi;
    uint64_t result, result_decimal;
    int8_t expresult;

    //get the exp bits of a with 11 effective bits, the exp bits of  2pi and 1/2pi which 1+frac bits has 32 effective bits
    expa =  expA  - 15 -10; 
    exptwopi = -29;//( 0x401921fb54442d18 >> 52 ) - 1023 - 52 + (53-32); use fp64 2pi to compute exp bits of 32 effective bits
    exprecip_twopi = -34;// ( 0x3fc45f306dc9c883 >> 52 ) - 1023 - 52 + (53-32); use fp64 1/2pi to compute exp bits of 32 effective bits


    //get the 32 bit effective bits of a and 2pi, 1/(2pi)
    siga = sigA + 0x400;
    sigtwopi = 0xC90FDAA2;//(( 0x401921fb54442d18 & 0xFFFFFFFFFFFFF ) + 0x10000000000000) >> (53-32); use fp64 2pi to compute 32 effective bits
    sigrecip_twopi = 0xA2F9836E;//(( 0x3fc45f306dc9c883 & 0xFFFFFFFFFFFFF ) + 0x10000000000000)>>(53-32);use fp64 1/2pi to compute 32 effective bits
    
    //a * 1/(2pi)
    result = (uint64_t)siga * sigrecip_twopi; 
    expresult = expa + exprecip_twopi;
    
    //get the decimal bits of a*(1/2pi)
    result_decimal = result & ((((uint64_t)1)<< (-expresult)) - 1 );

    //make the decimal bits to 32bits
    uint8_t zerobits = softfloat_countLeadingZeros64_( result_decimal );
    int8_t movebits = 64 - zerobits  - 32;
    if( movebits >= 0 )
    {
      result_decimal = result_decimal >> movebits;
      expresult += movebits;
    }
    else
    {
      result_decimal = result_decimal << (-movebits);
      expresult -= (-movebits);
    }
    
    //in order to move a to [-pi,pi], the decimal bits need to be in  [-0.5, 0.5], when the decimal bits > 0.5, decimal bits - 1
    if( ( -32 == expresult ) && ( ( result_decimal >> 31 ) == 1) )
    {
      result_decimal =  ((uint64_t)1 << 32 ) - result_decimal ;
      signA ^= 1;
      result_decimal = result_decimal << 1;
      expresult -= 1;
    }

    //in order to move a to [-pi/2,pi/2] for taylor series, when the decimal bits > 0.25, 0.5-decimal bits
    if( ( -33 == expresult ) && ( ( result_decimal >> 31 ) == 1) )
    {
      result_decimal = ((uint64_t)1 << 32 ) - result_decimal ;
    }

    siga = result_decimal & ((((uint64_t)1)<<32)-1);
    //decimalbits * 2pi
    result = (uint64_t)siga * sigtwopi; 
    expresult = expresult + exptwopi;

    //make the result to fp16
    zerobits = softfloat_countLeadingZeros64_( result ); 
    movebits = 64 - zerobits - 11;
    if( movebits >= 0 )
    {
      result = result >> movebits;
      expresult += movebits;
    }
    else
    {
      result = result << (-movebits);
      expresult -= (-movebits);
    }
    expresult += ( 15 + 10 );
    if( expresult <= 0 )
    {
      result = result >> (-expresult + 1);
      expresult = 0;    
    }

    a.v = ( signA << 15 ) + ( expresult << 10 ) + ( result & 0x3FF );
  }
  //taylor

  float16_t one = { 0x3c00 };//1
  float16_t recip_6 = { 0x3155 };//1/6
  float16_t recip_120 = { 0x2044 };//1/120
  float16_t recip_5040 = { 0xa80 };//1/5040  

  float16_t a2 = f16_mul( a, a );

  return f16_mul( a, f16_sub( one, 
  f16_mul( a2, f16_sub( recip_6, 
  f16_mul( a2, f16_sub( recip_120,
  f16_mul( a2, recip_5040 ) ) ) ) ) ) );

}