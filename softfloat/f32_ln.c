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
  float32_t recip_5 = { 0x3e4ccccd } ;//0.2
  float32_t recip_6 = { 0x3e2aaaab } ;//0.166666    
  float32_t frac;


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
    frac.v = frac.v - ( 1 << 23 );
  }
  frac = f32_sub( frac, one );
  expA = expA - 127 + 149; // expA-127 is the exp value, its minimum value is -149, so add 149 and use as index to look up table.
    
    
  //taylor series to compute ln(1+frac)
  uZ.f = f32_mul( frac, f32_sub( one, 
  f32_mul( frac, f32_sub( recip_2, 
  f32_mul( frac, f32_sub( recip_3, 
  f32_mul( frac, f32_sub( recip_4,
  f32_mul( frac, f32_sub( recip_5,
  f32_mul( frac, recip_6 ) ) ) ) ) ) ) ) ) ) );

  // use table look-up to compute exp*ln2
  float32_t ln2_n_1[] = { 
    0xc2ce8ed0, 0xc2cd2bec, 0xc2cbc908, 0xc2ca6623, 0xc2c9033f, 0xc2c7a05b, 0xc2c63d77, 0xc2c4da93, 
    0xc2c377ae, 0xc2c214ca, 0xc2c0b1e6, 0xc2bf4f02, 0xc2bdec1e, 0xc2bc893a, 0xc2bb2655, 0xc2b9c371, 
    0xc2b8608d, 0xc2b6fda9, 0xc2b59ac5, 0xc2b437e0, 0xc2b2d4fc, 0xc2b17218, 0xc2b00f34, 0xc2aeac50, 
    0xc2ad496b, 0xc2abe687, 0xc2aa83a3, 0xc2a920bf, 0xc2a7bddb, 0xc2a65af6, 0xc2a4f812, 0xc2a3952e, 
    0xc2a2324a, 0xc2a0cf66, 0xc29f6c82, 0xc29e099d, 0xc29ca6b9, 0xc29b43d5, 0xc299e0f1, 0xc2987e0d, 
    0xc2971b28, 0xc295b844, 0xc2945560, 0xc292f27c, 0xc2918f98, 0xc2902cb4, 0xc28ec9cf, 0xc28d66eb, 
    0xc28c0407, 0xc28aa123, 0xc2893e3f, 0xc287db5a, 0xc2867876, 0xc2851592, 0xc283b2ae, 0xc2824fca, 
    0xc280ece5, 0xc27f1402, 0xc27c4e3a, 0xc2798872, 0xc276c2a9, 0xc273fce1, 0xc2713719, 0xc26e7150, 
  };
  float32_t ln2_n_2[] = {     
    0xc26bab88, 0xc268e5c0, 0xc2661ff7, 0xc2635a2f, 0xc2609466, 0xc25dce9e, 0xc25b08d6, 0xc258430d, 
    0xc2557d45, 0xc252b77c, 0xc24ff1b4, 0xc24d2bec, 0xc24a6623, 0xc247a05b, 0xc244da93, 0xc24214ca, 
    0xc23f4f02, 0xc23c893a, 0xc239c371, 0xc236fda9, 0xc23437e0, 0xc2317218, 0xc22eac50, 0xc22be687, 
    0xc22920bf, 0xc2265af6, 0xc223952e, 0xc220cf66, 0xc21e099d, 0xc21b43d5, 0xc2187e0d, 0xc215b844, 
    0xc212f27c, 0xc2102cb4, 0xc20d66eb, 0xc20aa123, 0xc207db5a, 0xc2051592, 0xc2024fca, 0xc1ff1402, 
    0xc1f98872, 0xc1f3fce1, 0xc1ee7150, 0xc1e8e5c0, 0xc1e35a2f, 0xc1ddce9e, 0xc1d8430d, 0xc1d2b77c, 
    0xc1cd2bec, 0xc1c7a05b, 0xc1c214ca, 0xc1bc893a, 0xc1b6fda9, 0xc1b17218, 0xc1abe687, 0xc1a65af6, 
    0xc1a0cf66, 0xc19b43d5, 0xc195b844, 0xc1902cb4, 0xc18aa123, 0xc1851592, 0xc17f1402, 0xc173fce1, 
  };    
  float32_t ln2_n_3[] = {     
    0xc168e5c0, 0xc15dce9e, 0xc152b77c, 0xc147a05b, 0xc13c893a, 0xc1317218, 0xc1265af6, 0xc11b43d5, 
    0xc1102cb4, 0xc1051592, 0xc0f3fce1, 0xc0ddce9e, 0xc0c7a05b, 0xc0b17218, 0xc09b43d5, 0xc0851592, 
    0xc05dce9e, 0xc0317218, 0xc0051592, 0xbfb17218, 0xbf317218, 0x00000000, 0x3f317218, 0x3fb17218, 
    0x40051592, 0x40317218, 0x405dce9e, 0x40851592, 0x409b43d5, 0x40b17218, 0x40c7a05b, 0x40ddce9e, 
    0x40f3fce1, 0x41051592, 0x41102cb4, 0x411b43d5, 0x41265af6, 0x41317218, 0x413c893a, 0x4147a05b, 
    0x4152b77c, 0x415dce9e, 0x4168e5c0, 0x4173fce1, 0x417f1402, 0x41851592, 0x418aa123, 0x41902cb4, 
    0x4195b844, 0x419b43d5, 0x41a0cf66, 0x41a65af6, 0x41abe687, 0x41b17218, 0x41b6fda9, 0x41bc893a, 
    0x41c214ca, 0x41c7a05b, 0x41cd2bec, 0x41d2b77c, 0x41d8430d, 0x41ddce9e, 0x41e35a2f, 0x41e8e5c0, 
  };    
  float32_t ln2_n_4[] = {     
    0x41ee7150, 0x41f3fce1, 0x41f98872, 0x41ff1402, 0x42024fca, 0x42051592, 0x4207db5a, 0x420aa123, 
    0x420d66eb, 0x42102cb4, 0x4212f27c, 0x4215b844, 0x42187e0d, 0x421b43d5, 0x421e099d, 0x4220cf66, 
    0x4223952e, 0x42265af6, 0x422920bf, 0x422be687, 0x422eac50, 0x42317218, 0x423437e0, 0x4236fda9, 
    0x4239c371, 0x423c893a, 0x423f4f02, 0x424214ca, 0x4244da93, 0x4247a05b, 0x424a6623, 0x424d2bec, 
    0x424ff1b4, 0x4252b77c, 0x42557d45, 0x4258430d, 0x425b08d6, 0x425dce9e, 0x42609466, 0x42635a2f, 
    0x42661ff7, 0x4268e5c0, 0x426bab88, 0x426e7150, 0x42713719, 0x4273fce1, 0x4276c2a9, 0x42798872, 
    0x427c4e3a, 0x427f1402, 0x4280ece5, 0x42824fca, 0x4283b2ae, 0x42851592, 0x42867876, 0x4287db5a, 
    0x42893e3f, 0x428aa123, 0x428c0407, 0x428d66eb, 0x428ec9cf, 0x42902cb4, 0x42918f98, 0x4292f27c, 
  };    
  float32_t ln2_n_5[] = {     
    0x42945560, 0x4295b844, 0x42971b28, 0x42987e0d, 0x4299e0f1, 0x429b43d5, 0x429ca6b9, 0x429e099d, 
    0x429f6c82, 0x42a0cf66, 0x42a2324a, 0x42a3952e, 0x42a4f812, 0x42a65af6, 0x42a7bddb, 0x42a920bf, 
    0x42aa83a3, 0x42abe687, 0x42ad496b, 0x42aeac50, 0x42b00f34, 0x42b17218 
  };

  float32_t ln2_exp;
  if( expA < 64 )
  {
    ln2_exp = ln2_n_1[ expA ];
  }
  else if( expA < 128)
  {
    ln2_exp = ln2_n_2[ expA - 64 ];
  }
  else if( expA < 192)
  {
    ln2_exp = ln2_n_3[ expA - 128 ];
  }
  else if( expA < 256)
  {
    ln2_exp = ln2_n_4[ expA - 192 ];
  }   
  else
  {
    ln2_exp = ln2_n_5[ expA - 256 ];
  }   

  uZ.f = f32_add( ln2_exp, uZ.f ); 

  return uZ.f;

}