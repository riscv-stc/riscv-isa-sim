#include "platform.h"
#include "softfloat.h"
#include "internals.h"

extern uint_fast8_t softfloat_countLeadingZeros64_( uint64_t a );

bfloat16_t bf16_cos( bfloat16_t a )
{
  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast16_t sigA;

  uint_fast16_t uiZ;
  union ui16_bf16 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signBF16UI( uiA );
  expA = expBF16UI( uiA );
  sigA = fracBF16UI( uiA );

  //if expA = 0xff, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    uiZ = signA ? 0xffc0 : 0x7fc0;//-NaN or NaN;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //handle zero and subnormal number, return 1, which is the first item of taylor series
  if( 0 == expA )
  {
    uZ.ui = 0x3f80;
    return uZ.f;
  }

  //move a to [ -pi, pi ]
  //if a belongs to  (-inf, -pi) or (0, +inf) move a into [-pi/2,pi/2], else add pi/2, then use taylor series of sin ( which is more accurate than cos ) to compute.
  if( a.v > 0xbfc4 || a.v < 0x8000  )
  {
    //reciprocal of 2pi, from 2^-3....
    uint32_t recip_2pi[] = { 0xa2f9836e, 0x4e441529, 0xfc2757d1, 0xf534ddc0, 0xdb629599, 0x3c439041, 0xfe5163ab, 0xdebbc561, 0xb7246e3a, 0x424dd2e0, 0x6492eea,  0x9d1921c  };

    //compute the decimal part of a/(2pi)
    //if a is 2^k to 2^k-7, use 2^-k+6 to 2^-k-25 to compute a/(2pi), bigger than these bits won't produce decimal bits
    uint32_t siga = 0;
    int16_t expa;
    expA = expA - 127 - 7; //exp bits of integer mantissa bits
    sigA = sigA + 0x80; //integer of 1+frac bits

    int16_t exp_no;
    exp_no =  -expA + 6 -7;//-k+6 , (expA + 7) = k
    if( exp_no > -3 )   //the biggest bits of 1/(2pi) is -3
    {
      exp_no = -3;
    }
    int16_t no = ( -3 - exp_no ) / 32;
    int16_t end = -34 - 32 * no;
    int16_t bits = exp_no - end + 1;

    siga = ( recip_2pi[ no ] << (32 -bits) );
    siga += ( recip_2pi[ no + 1 ] >> bits ) & ( ( (uint32_t)1 << ( 32-bits ) ) - 1 );
    expa = exp_no - 31;

    uint64_t sigrema;
    int16_t exprema;

    //a *  2^-k+6 to 2^-k-25 bits of 1/(2pi) 
    sigrema = (uint64_t)siga * sigA ;
    exprema = expa + expA;


    if( -exprema < 64 && -exprema > 0 )
    {
      //get the decimal bits of result
      sigrema = sigrema & ((((uint64_t)1) << (-exprema)) - 1 );
    }

    //+pi/2(0.25 here) use taylor series of sin to compute cos
    uint64_t sigquarter = (uint64_t)1 << 62;
    int16_t expquarter = -64;
    if( exprema > expquarter ) 
    {
      sigrema = sigrema << ( exprema - expquarter );
    }
    else
    {
      sigrema = sigrema >> ( expquarter - exprema );
    }
    exprema = -64;
    if( signA )
    {
      if( sigrema > sigquarter )
      {
        sigrema -= sigquarter;
      }
      else
      {
        sigrema = sigquarter - sigrema;
        signA = 0;
      }  
    }
    else
    {
      sigrema += sigquarter;
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

    //get the bfp16 remainder result
    zerobits = softfloat_countLeadingZeros64_( sigrema );
    movebits = 64 - zerobits - 8;
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
    exprema += ( 127 + 7 );
    if( exprema <= 0 )
    {
      sigrema = sigrema >> (-exprema + 1);
      exprema = 0;    
    }

    a.v = ( signA << 15 ) + ( exprema << 7 ) + ( sigrema & 0x7F );  
  }
  else
  {
    bfloat16_t halfpi = { 0x3fc9 };
    a = bf16_add( a, halfpi );
  }

  //taylor
  bfloat16_t onebf = { 0x3f80 };//1
  bfloat16_t recip_6 = { 0x3e2b };//1/6
  bfloat16_t recip_120 = { 0x3c09 };//1/120
  bfloat16_t recip_5040 = { 0x3950 };//1/5040  

  bfloat16_t a2 = bf16_mul( a, a );

  uZ.f = bf16_mul( a, bf16_sub( onebf, 
  bf16_mul( a2, bf16_sub( recip_6, 
  bf16_mul( a2, bf16_sub( recip_120,
  bf16_mul( a2, recip_5040 ) ) ) ) ) ) );

  return uZ.f;

}