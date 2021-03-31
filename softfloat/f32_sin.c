#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

extern uint_fast8_t softfloat_countLeadingZeros64_( uint64_t a );

float32_t f32_sin( float32_t a )
{
  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast32_t expA;
  uint_fast32_t sigA;

  uint_fast32_t uiZ;
  union ui32_f32 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF32UI( uiA );
  expA = expF32UI( uiA );
  sigA = fracF32UI( uiA );

  //if expA = 0xff, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    uiZ = signA ? 0xffc00000 : 0x7fc00000;//-NaN or NaN;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //zero and subnormal number, just return itself
  if( 0x0 == expA )
  {
    return a;
  }

  //move a to [ -pi, pi ]
  //if a > pi/2, move a into [-pi/2,pi/2],else just use taylor series to compute.
  if( ( a.v & 0x7FFFFFFF ) > 0x3fc90fda )
  {
    //reciprocal of 2pi, from 2^-3....
    uint32_t recip_2pi[] = { 0xa2f9836e, 0x4e441529, 0xfc2757d1, 0xf534ddc0, 0xdb629599, 0x3c439041, 0xfe5163ab, 0xdebbc561, 0xb7246e3a, 0x424dd2e0, 0x6492eea,  0x9d1921c  };

    //compute the decimal part of a/(2pi)
    //if a is from 2^k to 2^k-23, use 2^-k+22 to 2^-k-9, 2^-k-10 to 2^-k-41 bits of 1/(2pi)to compute a/(2pi), bigger than these bits won't produce decimal bits
    uint32_t siga = 0, sigb = 0;
    int16_t expa, expb;
    expA = expA - 127 - 23; //exp bits of integer mantissa bits
    sigA = sigA + 0x800000; //integer of 1+frac bits

    int16_t exp_no;
    exp_no =  -expA + 22 - 23;//-k+22 , (expA + 23) = k
    if( exp_no > -3 ) //the biggest bits of 1/(2pi) is -3
    {
      exp_no = -3;
    }
    int16_t no = ( -3 - exp_no ) / 32;
    int16_t end = -34 - 32 * no;
    int16_t bits = exp_no - end + 1;

    siga = ( recip_2pi[ no ] << (32 -bits) );
    siga += ( recip_2pi[ no + 1 ] >> bits ) & ( ( (uint32_t)1 << ( 32-bits ) ) - 1 );
    sigb = ( recip_2pi[ no + 1 ] << (32 -bits) );
    sigb += ( recip_2pi[ no + 2 ] >>( bits ) ) & ( ( (uint32_t)1 << ( 32-bits ) ) - 1 );
    expa = exp_no - 31;
    expb = expa - 32;

    uint64_t sigrema;
    int16_t exprema;

    //a *  2^-k+22 to 2^-k-9 bits of 1/(2pi) 
    sigrema = (uint64_t)siga * sigA ;
    exprema = expa + expA;


    if( -exprema < 64 && -exprema > 0 )
    {
      //get the decimal bits of result
      sigrema = sigrema & ((((uint64_t)1) << (-exprema)) - 1 );
    }

    uint64_t sigremb;
    int16_t expremb;
    //a *  2^-k-10 to 2^-k-41 bits of 1/(2pi)
    sigremb = (uint64_t)sigb * sigA ;
    expremb = expb + expA;

    //rema + remb
    sigremb = sigremb >> ( exprema - expremb );
    sigrema += sigremb;

    if( -exprema < 64 && -exprema > 0 )
    {
      //get the decimal bits of result
      sigrema = sigrema & ((((uint64_t)1) << (-exprema)) - 1 );
    }

    //make the decimal bits to 32bits
    uint8_t zerobits = softfloat_countLeadingZeros64_( sigrema );
    int8_t movebits = 64 - zerobits  - 32;
    if( movebits >= 0 )
    {
      sigrema = sigrema >> movebits;
      exprema += movebits;
    }
    else
    {
      sigrema = sigrema << (-movebits);
      exprema -= (-movebits);
    }
    
    //in order to move a to [-pi,pi], the decimal bits need to be in  [-0.5, 0.5], when the decimal bits > 0.5, decimal bits - 1
    if( ( -32 == exprema ) && ( ( sigrema >> 31 ) == 1) )
    {
      sigrema = ((uint64_t)1 << 32 ) - sigrema ;
      signA ^= 1;
      sigrema = sigrema << 1;
      exprema -= 1;
    }

    //in order to move a to [-pi/2,pi/2] for taylor series, when the decimal bits > 0.25, 0.5-decimal bits
    if( ( -33 == exprema ) && ( ( sigrema >> 31 ) == 1) )
    {
      sigrema = ((uint64_t)1 << 32 ) - sigrema ;
    }

    siga = sigrema & ((((uint64_t)1)<<32)-1);

    //decimal bits * 2pi
    uint32_t sigtwopi = 0xC90FDAA2;//(( 0x401921fb54442d18 & 0xFFFFFFFFFFFFF ) + 0x10000000000000) >> (53-32); use fp64 2pi to compute 32 effective bits
    int16_t  exptwopi = -29;//( 0x401921fb54442d18 >> 52 ) - 1023 - 52 + (53-32); use fp64 2pi to compute exp bits of 32 effective bits
    sigrema = (uint64_t)siga * sigtwopi; 
    exprema = exprema + exptwopi;

    //get the fp32 remainder result
    zerobits = softfloat_countLeadingZeros64_( sigrema );
    movebits = 64 - zerobits - 24;
    if( movebits >= 0 )
    {
      sigrema = sigrema >> movebits;
      exprema += movebits;
    }
    else
    {
      sigrema = sigrema << (-movebits);
      exprema -= (-movebits);
    }
    exprema += ( 127 + 23 );
    if( exprema <= 0 )
    {
      sigrema = sigrema >> (-exprema + 1);
      exprema = 0;    
    }

    a.v = ( signA << 31 ) + ( exprema << 23 ) + ( sigrema & 0x7FFFFF ); 
  }

  //taylor
  float32_t one = { 0x3f800000 };//1
  float32_t recip_6 = { 0x3e2aaaab };//1/6
  float32_t recip_120 = { 0x3c088889 };//1/120
  float32_t recip_5040 = { 0x39500d01 };//1/5040  

  float32_t a2 = f32_mul( a, a );

  uZ.f = f32_mul( a, f32_sub( one, 
  f32_mul( a2, f32_sub( recip_6, 
  f32_mul( a2, f32_sub( recip_120,
  f32_mul( a2, recip_5040 ) ) ) ) ) ) );

  return uZ.f;

}