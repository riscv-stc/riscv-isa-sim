#include "platform.h"
#include "softfloat.h"
#include "internals.h"

bfloat16_t bf16_ln( bfloat16_t a )
{

  union ui16_bf16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast16_t sigA;
  uint_fast16_t uiZ;
  union ui16_bf16 uZ;

  bfloat16_t one = { 0x3f80 } ;//1
  bfloat16_t recip_2 = { 0x3f00 }  ;//0.5
  bfloat16_t recip_3 = { 0x3eab } ;//0.33333333333
  bfloat16_t recip_4 = { 0x3e80 } ;//0.25
  bfloat16_t ln2 = { 0x3f31 };
  bfloat16_t exp, frac;


  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signBF16UI( uiA );
  expA = expBF16UI( uiA );
  sigA = fracBF16UI( uiA );

  //if negative number
  if( 1 == signA )
  {
    //-0
    if( ( 0 == sigA ) && ( 0 == expA ) )
    {
      uiZ = 0xff80; //-inf
      uZ.ui = uiZ;
      return uZ.f;
    }

    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = 0xffc0;//-NaN;
    uZ.ui = uiZ;
    return uZ.f;
  }

  //if expA = 0xFF, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = 0x7fc0;//NaN;
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
      uiZ = 0xff80;
      uZ.ui = uiZ;
      return uZ.f;
  }

   
  //ln(a)=log2(a)/log2(e) a=2^exp*(1+frac) log2(a)= exp + log2(1+frac) log2(1+frac)= ln(1+frac)/ln2   
  //so ln(a)= (exp+ln(1+frac)/ln2)/log2(e)=exp*ln2 + ln(1+frac)
  if( 0 == expA )
  {//subnormal number

    int8_t zerobits = softfloat_countLeadingZeros16( sigA );
    int8_t movebits = 8- ( 16- zerobits );
    sigA = ( sigA << movebits ) & 0x7F;
    expA = 1 - movebits;
  }

  frac.v = sigA + 0x3f80;
  if( frac.v > 0x3faa  )//4/3
  {//frac/2
    expA += 1;
    frac = bf16_mul( frac, recip_2 );
  }
  frac = bf16_sub( frac, one );
  exp = i16_to_bf16( expA - 127 ); 
    
    
  //taylor series
  uZ.f = bf16_mul( frac, bf16_sub( one, 
  bf16_mul( frac, bf16_sub( recip_2, 
  bf16_mul( frac, bf16_sub( recip_3, bf16_mul( recip_4, frac ) )) ) ) ) );

  uZ.f = bf16_add( bf16_mul( ln2, exp ), uZ.f ); 

  return uZ.f;

}