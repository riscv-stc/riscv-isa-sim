#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

float16_t f16_ln( float16_t a )
{

  union ui16_f16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast16_t sigA;
  uint_fast16_t uiZ;
  union ui16_f16 uZ;

  float16_t one = { 0x3c00 } ;//1
  float16_t recip_2 = { 0x3800 }  ;//0.5
  float16_t recip_3 = { 0x3555 } ;//0.33333333333
  float16_t recip_4 = { 0x3400 } ;//0.25
  float16_t ln2 = { 0x398c };
  float16_t exp, frac;



  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF16UI( uiA );
  expA = expF16UI( uiA );
  sigA = fracF16UI( uiA );

  //if negative number
  if( 1 == signA )
  {
    //-0
    if( ( 0 == sigA ) && ( 0 == expA ) )
    {
      uiZ = 0xfc00; //-inf
      uZ.ui = uiZ;
      return uZ.f;
    }

    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = 0xfe00;//-NaN;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if expA = 0x1f, handle the NaN and infinite number
  if( 0x1F == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = 0x7e00;//NaN;
      uZ.ui = uiZ;
      return uZ.f;
    }
    //positive infinite number input
    return a;
  }

  //if 0 
  if( ( 0 == expA )  &&  ( sigA == 0 ))
  {
    //if +0, return -inf
      uiZ = 0xfc00;
      uZ.ui = uiZ;
      return uZ.f;
  }

   
  //ln(a)=log2(a)/log2(e) a=2^exp*(1+frac) log2(a)= exp + log2(1+frac) log2(1+frac)= ln(1+frac)/ln2   
  //so ln(a)= (exp+ln(1+frac)/ln2)/log2(e)=exp*ln2 + ln(1+frac)
  if( 0 == expA )
  {//subnormal number

    int8_t zerobits = softfloat_countLeadingZeros16( sigA );
    int8_t movebits = 11- ( 16- zerobits );
    sigA = ( sigA << movebits ) & 0x3FF;
    expA = 1 - movebits;
  }

  frac.v = sigA + 0x3C00;
  if( frac.v > 0x3d55  )//4/3
  {//frac/2
    expA += 1;
    frac = f16_mul( frac, recip_2 );
  }
  frac = f16_sub( frac, one );
  exp = i8_to_f16( expA - 15 ); 
    
    
  //taylor series
  uZ.f = f16_mul( frac, f16_sub( one, 
  f16_mul( frac, f16_sub( recip_2, 
  f16_mul( frac, f16_sub( recip_3, f16_mul( recip_4, frac ) )) ) ) ) );

  uZ.f = f16_add( f16_mul( ln2, exp ), uZ.f ); 

  //makeup table
  uint16_t index = ( a.v >> 6 ) & 0x1FF;
  if( index >= 200 && index < 256 )
  {
    index -= 200;
    uint16_t makeup[ ] = {1, 1, 1, 0, 2, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 2, 1, 2, 2, 0, 1, 1, 1, 1, 1, 2, 2};
    uint16_t makeup_signal[] = { 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};
    uZ.ui = makeup_signal[index] ? uZ.ui + makeup[index] : uZ.ui - makeup[index];
  }

  return uZ.f;

}

