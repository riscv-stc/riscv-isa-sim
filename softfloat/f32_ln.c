#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

float32_t f32_ln( float32_t a )
{

  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast32_t sigA;
  uint_fast32_t uiZ;
  union ui32_f32 uZ;

  float32_t one = { 0x3f800000 } ;//1
  float32_t recip_2 = { 0x3f000000 }  ;//0.5
  float32_t recip_3 = { 0x3eaaaaab } ;//0.33333333333
  float32_t recip_4 = { 0x3e800000 } ;//0.25
  float32_t ln2 = { 0x3f317218 };
  float32_t exp, frac;


  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF32UI( uiA );
  expA = expF32UI( uiA );
  sigA = fracF32UI( uiA );

  //if negative number
  if( 1 == signA )
  {
    //-0
    if( ( 0 == sigA ) && ( 0 == expA ) )
    {
      uiZ = 0xff800000; //-inf
      uZ.ui = uiZ;
      return uZ.f;
    }

    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = 0xffc00000;//-NaN;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if expA = 0xFF, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = 0x7fc00000;//NaN;
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
      uiZ = 0xff800000;
      uZ.ui = uiZ;
      return uZ.f;
  }

   
  //ln(a)=log2(a)/log2(e) a=2^exp*(1+frac) log2(a)= exp + log2(1+frac) log2(1+frac)= ln(1+frac)/ln2   
  //so ln(a)= (exp+ln(1+frac)/ln2)/log2(e)=exp*ln2 + ln(1+frac)
  if( 0 == expA )
  {//subnormal number

    int8_t zerobits = softfloat_countLeadingZeros32( sigA );
    int8_t movebits = 24- ( 32- zerobits );
    sigA = ( sigA << movebits ) & 0x7FFFFF;
    expA = 1 - movebits;
  }

  frac.v = sigA + 0x3f800000;
  if( frac.v > 0x3faaaaab  )//4/3
  {//frac/2
    expA += 1;
    frac = f32_mul( frac, recip_2 );
  }
  frac = f32_sub( frac, one );
  exp = i32_to_f32( expA - 127 ); 
    
    
  //taylor series
  uZ.f = f32_mul( frac, f32_sub( one, 
  f32_mul( frac, f32_sub( recip_2, 
  f32_mul( frac, f32_sub( recip_3, f32_mul( recip_4, frac ) )) ) ) ) );

  uZ.f = f32_add( f32_mul( ln2, exp ), uZ.f ); 

  return uZ.f;

}