#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"
#include <stdio.h>

float32_t f32_sinh( float32_t a )
{

  union ui32_f32 uA;
  uint_fast32_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast32_t sigA;

  uint_fast32_t uiZ;
  union ui32_f32 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF32UI( uiA );
  expA = expF32UI( uiA );
  sigA = fracF32UI( uiA );

  //if expA = 0x1f, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xffc00000 : 0x7fc00000;//-NaN or NaN;
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0xff800000 : 0x7f800000;//-inf or inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( ( a.v & 0x7FFFFFFF ) >= 0x42b2d4fd )
  {
    uiZ = signA ? 0xff800000 : 0x7f800000;//-inf or inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  /** when x=nln2, the sinh(x) and all levels of deratives are easy to compute, 
   * so use the taylor series when x close to x0=nln2.
   */ 
  float32_t two, ln2_recip;
  two.v = 0x40000000; 
  ln2_recip.v = 0x3fb8aa3b; // 1/ln2=1.4426950408889634

  float32_t n = f32_roundToInt( f32_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2) 
  int32_t n_int = f32_to_i32( n, softfloat_round_near_maxMag, false );


  // use table look-up to compute n*ln2 and parameters of taylor series
  float32_t ln2_n_1[] = {
    0xc2b2d4fc, 0xc2b17218, 0xc2b00f34, 0xc2aeac50, 0xc2ad496b, 0xc2abe687, 0xc2aa83a3, 0xc2a920bf, 
    0xc2a7bddb, 0xc2a65af6, 0xc2a4f812, 0xc2a3952e, 0xc2a2324a, 0xc2a0cf66, 0xc29f6c82, 0xc29e099d, 
    0xc29ca6b9, 0xc29b43d5, 0xc299e0f1, 0xc2987e0d, 0xc2971b28, 0xc295b844, 0xc2945560, 0xc292f27c, 
    0xc2918f98, 0xc2902cb3, 0xc28ec9cf, 0xc28d66eb, 0xc28c0407, 0xc28aa123, 0xc2893e3f, 0xc287db5a, 
    0xc2867876, 0xc2851592, 0xc283b2ae, 0xc2824fca, 0xc280ece5, 0xc27f1402, 0xc27c4e3a, 0xc2798872, 
    0xc276c2a9, 0xc273fce1, 0xc2713719, 0xc26e7150, 0xc26bab88, 0xc268e5bf, 0xc2661ff7, 0xc2635a2f, 
    0xc2609466, 0xc25dce9e, 0xc25b08d6, 0xc258430d, 0xc2557d45, 0xc252b77c, 0xc24ff1b4, 0xc24d2bec, 
    0xc24a6623, 0xc247a05b, 0xc244da93, 0xc24214ca, 0xc23f4f02, 0xc23c8939, 0xc239c371, 0xc236fda9, 
  };
  float32_t ln2_n_2[] = {    
    0xc23437e0, 0xc2317218, 0xc22eac50, 0xc22be687, 0xc22920bf, 0xc2265af6, 0xc223952e, 0xc220cf66, 
    0xc21e099d, 0xc21b43d5, 0xc2187e0d, 0xc215b844, 0xc212f27c, 0xc2102cb3, 0xc20d66eb, 0xc20aa123, 
    0xc207db5a, 0xc2051592, 0xc2024fca, 0xc1ff1402, 0xc1f98872, 0xc1f3fce1, 0xc1ee7150, 0xc1e8e5bf, 
    0xc1e35a2f, 0xc1ddce9e, 0xc1d8430d, 0xc1d2b77c, 0xc1cd2bec, 0xc1c7a05b, 0xc1c214ca, 0xc1bc8939, 
    0xc1b6fda9, 0xc1b17218, 0xc1abe687, 0xc1a65af6, 0xc1a0cf66, 0xc19b43d5, 0xc195b844, 0xc1902cb3, 
    0xc18aa123, 0xc1851592, 0xc17f1402, 0xc173fce1, 0xc168e5bf, 0xc15dce9e, 0xc152b77c, 0xc147a05b, 
    0xc13c8939, 0xc1317218, 0xc1265af6, 0xc11b43d5, 0xc1102cb3, 0xc1051592, 0xc0f3fce1, 0xc0ddce9e, 
    0xc0c7a05b, 0xc0b17218, 0xc09b43d5, 0xc0851592, 0xc05dce9e, 0xc0317218, 0xc0051592, 0xbfb17218, 
  };
  float32_t ln2_n_3[] = {      
    0xbf317218, 0x00000000, 0x3f317218, 0x3fb17218, 0x40051592, 0x40317218, 0x405dce9e, 0x40851592, 
    0x409b43d5, 0x40b17218, 0x40c7a05b, 0x40ddce9e, 0x40f3fce1, 0x41051592, 0x41102cb3, 0x411b43d5, 
    0x41265af6, 0x41317218, 0x413c8939, 0x4147a05b, 0x4152b77c, 0x415dce9e, 0x4168e5bf, 0x4173fce1, 
    0x417f1402, 0x41851592, 0x418aa123, 0x41902cb3, 0x4195b844, 0x419b43d5, 0x41a0cf66, 0x41a65af6, 
    0x41abe687, 0x41b17218, 0x41b6fda9, 0x41bc8939, 0x41c214ca, 0x41c7a05b, 0x41cd2bec, 0x41d2b77c, 
    0x41d8430d, 0x41ddce9e, 0x41e35a2f, 0x41e8e5bf, 0x41ee7150, 0x41f3fce1, 0x41f98872, 0x41ff1402, 
    0x42024fca, 0x42051592, 0x4207db5a, 0x420aa123, 0x420d66eb, 0x42102cb3, 0x4212f27c, 0x4215b844, 
    0x42187e0d, 0x421b43d5, 0x421e099d, 0x4220cf66, 0x4223952e, 0x42265af6, 0x422920bf, 0x422be687, 
  };
  float32_t ln2_n_4[] = {      
    0x422eac50, 0x42317218, 0x423437e0, 0x4236fda9, 0x4239c371, 0x423c8939, 0x423f4f02, 0x424214ca, 
    0x4244da93, 0x4247a05b, 0x424a6623, 0x424d2bec, 0x424ff1b4, 0x4252b77c, 0x42557d45, 0x4258430d, 
    0x425b08d6, 0x425dce9e, 0x42609466, 0x42635a2f, 0x42661ff7, 0x4268e5bf, 0x426bab88, 0x426e7150, 
    0x42713719, 0x4273fce1, 0x4276c2a9, 0x42798872, 0x427c4e3a, 0x427f1402, 0x4280ece5, 0x42824fca, 
    0x4283b2ae, 0x42851592, 0x42867876, 0x4287db5a, 0x42893e3f, 0x428aa123, 0x428c0407, 0x428d66eb, 
    0x428ec9cf, 0x42902cb3, 0x42918f98, 0x4292f27c, 0x42945560, 0x4295b844, 0x42971b28, 0x42987e0d, 
    0x4299e0f1, 0x429b43d5, 0x429ca6b9, 0x429e099d, 0x429f6c82, 0x42a0cf66, 0x42a2324a, 0x42a3952e, 
    0x42a4f812, 0x42a65af6, 0x42a7bddb, 0x42a920bf, 0x42aa83a3, 0x42abe687, 0x42ad496b, 0x42aeac50, 
  };
  float32_t ln2_n_5[] = {      
    0x42b00f34, 0x42b17218, 0x42b2d4fc 
  };

  float32_t val_n_1[] = {
    0xff000000, 0xfe800000, 0xfe000000, 0xfd800000, 0xfd000000, 0xfc800000, 0xfc000000, 0xfb800000, 
    0xfb000000, 0xfa800000, 0xfa000000, 0xf9800000, 0xf9000000, 0xf8800000, 0xf8000000, 0xf7800000, 
    0xf7000000, 0xf6800000, 0xf6000000, 0xf5800000, 0xf5000000, 0xf4800000, 0xf4000000, 0xf3800000, 
    0xf3000000, 0xf2800000, 0xf2000000, 0xf1800000, 0xf1000000, 0xf0800000, 0xf0000000, 0xef800000, 
    0xef000000, 0xee800000, 0xee000000, 0xed800000, 0xed000000, 0xec800000, 0xec000000, 0xeb800000, 
    0xeb000000, 0xea800000, 0xea000000, 0xe9800000, 0xe9000000, 0xe8800000, 0xe8000000, 0xe7800000, 
    0xe7000000, 0xe6800000, 0xe6000000, 0xe5800000, 0xe5000000, 0xe4800000, 0xe4000000, 0xe3800000, 
    0xe3000000, 0xe2800000, 0xe2000000, 0xe1800000, 0xe1000000, 0xe0800000, 0xe0000000, 0xdf800000, 
  };
  float32_t val_n_2[] = {
    0xdf000000, 0xde800000, 0xde000000, 0xdd800000, 0xdd000000, 0xdc800000, 0xdc000000, 0xdb800000, 
    0xdb000000, 0xda800000, 0xda000000, 0xd9800000, 0xd9000000, 0xd8800000, 0xd8000000, 0xd7800000, 
    0xd7000000, 0xd6800000, 0xd6000000, 0xd5800000, 0xd5000000, 0xd4800000, 0xd4000000, 0xd3800000, 
    0xd3000000, 0xd2800000, 0xd2000000, 0xd1800000, 0xd1000000, 0xd0800000, 0xd0000000, 0xcf800000, 
    0xcf000000, 0xce800000, 0xce000000, 0xcd800000, 0xcd000000, 0xcc800000, 0xcc000000, 0xcb800000, 
    0xcb000000, 0xca800000, 0xca000000, 0xc9800000, 0xc9000000, 0xc8800000, 0xc8000000, 0xc7800000, 
    0xc7000000, 0xc6800000, 0xc6000000, 0xc5800000, 0xc5000000, 0xc47fffff, 0xc3fffffc, 0xc37ffff0, 
    0xc2ffffc0, 0xc27fff00, 0xc1fffc00, 0xc17ff000, 0xc0ffc000, 0xc07f0000, 0xbffc0000, 0xbf700000, 
  };
  float32_t val_n_3[] = {
    0xbec00000, 0x00000000, 0x3ec00000, 0x3f700000, 0x3ffc0000, 0x407f0000, 0x40ffc000, 0x417ff000, 
    0x41fffc00, 0x427fff00, 0x42ffffc0, 0x437ffff0, 0x43fffffc, 0x447fffff, 0x45000000, 0x45800000, 
    0x46000000, 0x46800000, 0x47000000, 0x47800000, 0x48000000, 0x48800000, 0x49000000, 0x49800000, 
    0x4a000000, 0x4a800000, 0x4b000000, 0x4b800000, 0x4c000000, 0x4c800000, 0x4d000000, 0x4d800000, 
    0x4e000000, 0x4e800000, 0x4f000000, 0x4f800000, 0x50000000, 0x50800000, 0x51000000, 0x51800000, 
    0x52000000, 0x52800000, 0x53000000, 0x53800000, 0x54000000, 0x54800000, 0x55000000, 0x55800000, 
    0x56000000, 0x56800000, 0x57000000, 0x57800000, 0x58000000, 0x58800000, 0x59000000, 0x59800000, 
    0x5a000000, 0x5a800000, 0x5b000000, 0x5b800000, 0x5c000000, 0x5c800000, 0x5d000000, 0x5d800000, 
  };
  float32_t val_n_4[] = {
    0x5e000000, 0x5e800000, 0x5f000000, 0x5f800000, 0x60000000, 0x60800000, 0x61000000, 0x61800000, 
    0x62000000, 0x62800000, 0x63000000, 0x63800000, 0x64000000, 0x64800000, 0x65000000, 0x65800000, 
    0x66000000, 0x66800000, 0x67000000, 0x67800000, 0x68000000, 0x68800000, 0x69000000, 0x69800000, 
    0x6a000000, 0x6a800000, 0x6b000000, 0x6b800000, 0x6c000000, 0x6c800000, 0x6d000000, 0x6d800000, 
    0x6e000000, 0x6e800000, 0x6f000000, 0x6f800000, 0x70000000, 0x70800000, 0x71000000, 0x71800000, 
    0x72000000, 0x72800000, 0x73000000, 0x73800000, 0x74000000, 0x74800000, 0x75000000, 0x75800000, 
    0x76000000, 0x76800000, 0x77000000, 0x77800000, 0x78000000, 0x78800000, 0x79000000, 0x79800000, 
    0x7a000000, 0x7a800000, 0x7b000000, 0x7b800000, 0x7c000000, 0x7c800000, 0x7d000000, 0x7d800000, 
  };
  float32_t val_n_5[] = {
    0x7e000000, 0x7e800000, 0x7f000000,    
  };

  float32_t der_1_1[] = {
    0x7f000000, 0x7e800000, 0x7e000000, 0x7d800000, 0x7d000000, 0x7c800000, 0x7c000000, 0x7b800000, 
    0x7b000000, 0x7a800000, 0x7a000000, 0x79800000, 0x79000000, 0x78800000, 0x78000000, 0x77800000, 
    0x77000000, 0x76800000, 0x76000000, 0x75800000, 0x75000000, 0x74800000, 0x74000000, 0x73800000, 
    0x73000000, 0x72800000, 0x72000000, 0x71800000, 0x71000000, 0x70800000, 0x70000000, 0x6f800000, 
    0x6f000000, 0x6e800000, 0x6e000000, 0x6d800000, 0x6d000000, 0x6c800000, 0x6c000000, 0x6b800000, 
    0x6b000000, 0x6a800000, 0x6a000000, 0x69800000, 0x69000000, 0x68800000, 0x68000000, 0x67800000, 
    0x67000000, 0x66800000, 0x66000000, 0x65800000, 0x65000000, 0x64800000, 0x64000000, 0x63800000, 
    0x63000000, 0x62800000, 0x62000000, 0x61800000, 0x61000000, 0x60800000, 0x60000000, 0x5f800000, 
  };
  float32_t der_1_2[] = {
    0x5f000000, 0x5e800000, 0x5e000000, 0x5d800000, 0x5d000000, 0x5c800000, 0x5c000000, 0x5b800000, 
    0x5b000000, 0x5a800000, 0x5a000000, 0x59800000, 0x59000000, 0x58800000, 0x58000000, 0x57800000, 
    0x57000000, 0x56800000, 0x56000000, 0x55800000, 0x55000000, 0x54800000, 0x54000000, 0x53800000, 
    0x53000000, 0x52800000, 0x52000000, 0x51800000, 0x51000000, 0x50800000, 0x50000000, 0x4f800000, 
    0x4f000000, 0x4e800000, 0x4e000000, 0x4d800000, 0x4d000000, 0x4c800000, 0x4c000000, 0x4b800000, 
    0x4b000000, 0x4a800000, 0x4a000000, 0x49800000, 0x49000000, 0x48800000, 0x48000000, 0x47800000, 
    0x47000000, 0x46800000, 0x46000000, 0x45800000, 0x45000000, 0x44800000, 0x44000002, 0x43800008, 
    0x43000020, 0x42800080, 0x42000200, 0x41800800, 0x41002000, 0x40808000, 0x40020000, 0x3f880000, 
  };
  float32_t der_1_3[] = {
    0x3f200000, 0x3f000000, 0x3f200000, 0x3f880000, 0x40020000, 0x40808000, 0x41002000, 0x41800800, 
    0x42000200, 0x42800080, 0x43000020, 0x43800008, 0x44000002, 0x44800000, 0x45000000, 0x45800000, 
    0x46000000, 0x46800000, 0x47000000, 0x47800000, 0x48000000, 0x48800000, 0x49000000, 0x49800000, 
    0x4a000000, 0x4a800000, 0x4b000000, 0x4b800000, 0x4c000000, 0x4c800000, 0x4d000000, 0x4d800000, 
    0x4e000000, 0x4e800000, 0x4f000000, 0x4f800000, 0x50000000, 0x50800000, 0x51000000, 0x51800000, 
    0x52000000, 0x52800000, 0x53000000, 0x53800000, 0x54000000, 0x54800000, 0x55000000, 0x55800000, 
    0x56000000, 0x56800000, 0x57000000, 0x57800000, 0x58000000, 0x58800000, 0x59000000, 0x59800000, 
    0x5a000000, 0x5a800000, 0x5b000000, 0x5b800000, 0x5c000000, 0x5c800000, 0x5d000000, 0x5d800000, 
  };
  float32_t der_1_4[] = {
    0x5e000000, 0x5e800000, 0x5f000000, 0x5f800000, 0x60000000, 0x60800000, 0x61000000, 0x61800000, 
    0x62000000, 0x62800000, 0x63000000, 0x63800000, 0x64000000, 0x64800000, 0x65000000, 0x65800000, 
    0x66000000, 0x66800000, 0x67000000, 0x67800000, 0x68000000, 0x68800000, 0x69000000, 0x69800000, 
    0x6a000000, 0x6a800000, 0x6b000000, 0x6b800000, 0x6c000000, 0x6c800000, 0x6d000000, 0x6d800000, 
    0x6e000000, 0x6e800000, 0x6f000000, 0x6f800000, 0x70000000, 0x70800000, 0x71000000, 0x71800000, 
    0x72000000, 0x72800000, 0x73000000, 0x73800000, 0x74000000, 0x74800000, 0x75000000, 0x75800000, 
    0x76000000, 0x76800000, 0x77000000, 0x77800000, 0x78000000, 0x78800000, 0x79000000, 0x79800000, 
    0x7a000000, 0x7a800000, 0x7b000000, 0x7b800000, 0x7c000000, 0x7c800000, 0x7d000000, 0x7d800000, 
  };
  float32_t der_1_5[] = {
    0x7e000000, 0x7e800000, 0x7f000000,
  };

  float32_t der_2_1[] = {     
    0xfe800000, 0xfe000000, 0xfd800000, 0xfd000000, 0xfc800000, 0xfc000000, 0xfb800000, 0xfb000000, 
    0xfa800000, 0xfa000000, 0xf9800000, 0xf9000000, 0xf8800000, 0xf8000000, 0xf7800000, 0xf7000000, 
    0xf6800000, 0xf6000000, 0xf5800000, 0xf5000000, 0xf4800000, 0xf4000000, 0xf3800000, 0xf3000000, 
    0xf2800000, 0xf2000000, 0xf1800000, 0xf1000000, 0xf0800000, 0xf0000000, 0xef800000, 0xef000000, 
    0xee800000, 0xee000000, 0xed800000, 0xed000000, 0xec800000, 0xec000000, 0xeb800000, 0xeb000000, 
    0xea800000, 0xea000000, 0xe9800000, 0xe9000000, 0xe8800000, 0xe8000000, 0xe7800000, 0xe7000000, 
    0xe6800000, 0xe6000000, 0xe5800000, 0xe5000000, 0xe4800000, 0xe4000000, 0xe3800000, 0xe3000000, 
    0xe2800000, 0xe2000000, 0xe1800000, 0xe1000000, 0xe0800000, 0xe0000000, 0xdf800000, 0xdf000000, 
  };
  float32_t der_2_2[] = {
    0xde800000, 0xde000000, 0xdd800000, 0xdd000000, 0xdc800000, 0xdc000000, 0xdb800000, 0xdb000000, 
    0xda800000, 0xda000000, 0xd9800000, 0xd9000000, 0xd8800000, 0xd8000000, 0xd7800000, 0xd7000000, 
    0xd6800000, 0xd6000000, 0xd5800000, 0xd5000000, 0xd4800000, 0xd4000000, 0xd3800000, 0xd3000000, 
    0xd2800000, 0xd2000000, 0xd1800000, 0xd1000000, 0xd0800000, 0xd0000000, 0xcf800000, 0xcf000000, 
    0xce800000, 0xce000000, 0xcd800000, 0xcd000000, 0xcc800000, 0xcc000000, 0xcb800000, 0xcb000000, 
    0xca800000, 0xca000000, 0xc9800000, 0xc9000000, 0xc8800000, 0xc8000000, 0xc7800000, 0xc7000000, 
    0xc6800000, 0xc6000000, 0xc5800000, 0xc5000000, 0xc4800000, 0xc3ffffff, 0xc37ffffc, 0xc2fffff0, 
    0xc27fffc0, 0xc1ffff00, 0xc17ffc00, 0xc0fff000, 0xc07fc000, 0xbfff0000, 0xbf7c0000, 0xbef00000, 
  };
  float32_t der_2_3[] = {
    0xbe400000, 0x00000000, 0x3e400000, 0x3ef00000, 0x3f7c0000, 0x3fff0000, 0x407fc000, 0x40fff000, 
    0x417ffc00, 0x41ffff00, 0x427fffc0, 0x42fffff0, 0x437ffffc, 0x43ffffff, 0x44800000, 0x45000000, 
    0x45800000, 0x46000000, 0x46800000, 0x47000000, 0x47800000, 0x48000000, 0x48800000, 0x49000000, 
    0x49800000, 0x4a000000, 0x4a800000, 0x4b000000, 0x4b800000, 0x4c000000, 0x4c800000, 0x4d000000, 
    0x4d800000, 0x4e000000, 0x4e800000, 0x4f000000, 0x4f800000, 0x50000000, 0x50800000, 0x51000000, 
    0x51800000, 0x52000000, 0x52800000, 0x53000000, 0x53800000, 0x54000000, 0x54800000, 0x55000000, 
    0x55800000, 0x56000000, 0x56800000, 0x57000000, 0x57800000, 0x58000000, 0x58800000, 0x59000000, 
    0x59800000, 0x5a000000, 0x5a800000, 0x5b000000, 0x5b800000, 0x5c000000, 0x5c800000, 0x5d000000, 
  };
  float32_t der_2_4[] = {
    0x5d800000, 0x5e000000, 0x5e800000, 0x5f000000, 0x5f800000, 0x60000000, 0x60800000, 0x61000000, 
    0x61800000, 0x62000000, 0x62800000, 0x63000000, 0x63800000, 0x64000000, 0x64800000, 0x65000000, 
    0x65800000, 0x66000000, 0x66800000, 0x67000000, 0x67800000, 0x68000000, 0x68800000, 0x69000000, 
    0x69800000, 0x6a000000, 0x6a800000, 0x6b000000, 0x6b800000, 0x6c000000, 0x6c800000, 0x6d000000, 
    0x6d800000, 0x6e000000, 0x6e800000, 0x6f000000, 0x6f800000, 0x70000000, 0x70800000, 0x71000000, 
    0x71800000, 0x72000000, 0x72800000, 0x73000000, 0x73800000, 0x74000000, 0x74800000, 0x75000000, 
    0x75800000, 0x76000000, 0x76800000, 0x77000000, 0x77800000, 0x78000000, 0x78800000, 0x79000000, 
    0x79800000, 0x7a000000, 0x7a800000, 0x7b000000, 0x7b800000, 0x7c000000, 0x7c800000, 0x7d000000, 
  };
  float32_t der_2_5[] = {
    0x7d800000, 0x7e000000, 0x7e800000, 
  };

  float32_t der_3_1[] = {    
    0x7daaaaab, 0x7d2aaaab, 0x7caaaaab, 0x7c2aaaab, 0x7baaaaab, 0x7b2aaaab, 0x7aaaaaab, 0x7a2aaaab, 
    0x79aaaaab, 0x792aaaab, 0x78aaaaab, 0x782aaaab, 0x77aaaaab, 0x772aaaab, 0x76aaaaab, 0x762aaaab, 
    0x75aaaaab, 0x752aaaab, 0x74aaaaab, 0x742aaaab, 0x73aaaaab, 0x732aaaab, 0x72aaaaab, 0x722aaaab, 
    0x71aaaaab, 0x712aaaab, 0x70aaaaab, 0x702aaaab, 0x6faaaaab, 0x6f2aaaab, 0x6eaaaaab, 0x6e2aaaab, 
    0x6daaaaab, 0x6d2aaaab, 0x6caaaaab, 0x6c2aaaab, 0x6baaaaab, 0x6b2aaaab, 0x6aaaaaab, 0x6a2aaaab, 
    0x69aaaaab, 0x692aaaab, 0x68aaaaab, 0x682aaaab, 0x67aaaaab, 0x672aaaab, 0x66aaaaab, 0x662aaaab, 
    0x65aaaaab, 0x652aaaab, 0x64aaaaab, 0x642aaaab, 0x63aaaaab, 0x632aaaab, 0x62aaaaab, 0x622aaaab, 
    0x61aaaaab, 0x612aaaab, 0x60aaaaab, 0x602aaaab, 0x5faaaaab, 0x5f2aaaab, 0x5eaaaaab, 0x5e2aaaab, 
  };
  float32_t der_3_2[] = {
    0x5daaaaab, 0x5d2aaaab, 0x5caaaaab, 0x5c2aaaab, 0x5baaaaab, 0x5b2aaaab, 0x5aaaaaab, 0x5a2aaaab, 
    0x59aaaaab, 0x592aaaab, 0x58aaaaab, 0x582aaaab, 0x57aaaaab, 0x572aaaab, 0x56aaaaab, 0x562aaaab, 
    0x55aaaaab, 0x552aaaab, 0x54aaaaab, 0x542aaaab, 0x53aaaaab, 0x532aaaab, 0x52aaaaab, 0x522aaaab, 
    0x51aaaaab, 0x512aaaab, 0x50aaaaab, 0x502aaaab, 0x4faaaaab, 0x4f2aaaab, 0x4eaaaaab, 0x4e2aaaab, 
    0x4daaaaab, 0x4d2aaaab, 0x4caaaaab, 0x4c2aaaab, 0x4baaaaab, 0x4b2aaaab, 0x4aaaaaab, 0x4a2aaaab, 
    0x49aaaaab, 0x492aaaab, 0x48aaaaab, 0x482aaaab, 0x47aaaaab, 0x472aaaab, 0x46aaaaab, 0x462aaaab, 
    0x45aaaaab, 0x452aaaab, 0x44aaaaab, 0x442aaaab, 0x43aaaaab, 0x432aaaab, 0x42aaaaad, 0x422aaab5, 
    0x41aaaad5, 0x412aab55, 0x40aaad55, 0x402ab555, 0x3faad555, 0x3f2b5555, 0x3ead5555, 0x3e355555, 
  };
  float32_t der_3_3[] = {
    0x3dd55555, 0x3daaaaab, 0x3dd55555, 0x3e355555, 0x3ead5555, 0x3f2b5555, 0x3faad555, 0x402ab555, 
    0x40aaad55, 0x412aab55, 0x41aaaad5, 0x422aaab5, 0x42aaaaad, 0x432aaaab, 0x43aaaaab, 0x442aaaab, 
    0x44aaaaab, 0x452aaaab, 0x45aaaaab, 0x462aaaab, 0x46aaaaab, 0x472aaaab, 0x47aaaaab, 0x482aaaab, 
    0x48aaaaab, 0x492aaaab, 0x49aaaaab, 0x4a2aaaab, 0x4aaaaaab, 0x4b2aaaab, 0x4baaaaab, 0x4c2aaaab, 
    0x4caaaaab, 0x4d2aaaab, 0x4daaaaab, 0x4e2aaaab, 0x4eaaaaab, 0x4f2aaaab, 0x4faaaaab, 0x502aaaab, 
    0x50aaaaab, 0x512aaaab, 0x51aaaaab, 0x522aaaab, 0x52aaaaab, 0x532aaaab, 0x53aaaaab, 0x542aaaab, 
    0x54aaaaab, 0x552aaaab, 0x55aaaaab, 0x562aaaab, 0x56aaaaab, 0x572aaaab, 0x57aaaaab, 0x582aaaab, 
    0x58aaaaab, 0x592aaaab, 0x59aaaaab, 0x5a2aaaab, 0x5aaaaaab, 0x5b2aaaab, 0x5baaaaab, 0x5c2aaaab, 
  };
  float32_t der_3_4[] = {
    0x5caaaaab, 0x5d2aaaab, 0x5daaaaab, 0x5e2aaaab, 0x5eaaaaab, 0x5f2aaaab, 0x5faaaaab, 0x602aaaab, 
    0x60aaaaab, 0x612aaaab, 0x61aaaaab, 0x622aaaab, 0x62aaaaab, 0x632aaaab, 0x63aaaaab, 0x642aaaab, 
    0x64aaaaab, 0x652aaaab, 0x65aaaaab, 0x662aaaab, 0x66aaaaab, 0x672aaaab, 0x67aaaaab, 0x682aaaab, 
    0x68aaaaab, 0x692aaaab, 0x69aaaaab, 0x6a2aaaab, 0x6aaaaaab, 0x6b2aaaab, 0x6baaaaab, 0x6c2aaaab, 
    0x6caaaaab, 0x6d2aaaab, 0x6daaaaab, 0x6e2aaaab, 0x6eaaaaab, 0x6f2aaaab, 0x6faaaaab, 0x702aaaab, 
    0x70aaaaab, 0x712aaaab, 0x71aaaaab, 0x722aaaab, 0x72aaaaab, 0x732aaaab, 0x73aaaaab, 0x742aaaab, 
    0x74aaaaab, 0x752aaaab, 0x75aaaaab, 0x762aaaab, 0x76aaaaab, 0x772aaaab, 0x77aaaaab, 0x782aaaab, 
    0x78aaaaab, 0x792aaaab, 0x79aaaaab, 0x7a2aaaab, 0x7aaaaaab, 0x7b2aaaab, 0x7baaaaab, 0x7c2aaaab, 
  };
  float32_t der_3_5[] = {
    0x7caaaaab, 0x7d2aaaab, 0x7daaaaab, 
  };

  float32_t der_4_1[] = {    
    0xfcaaaaab, 0xfc2aaaab, 0xfbaaaaab, 0xfb2aaaab, 0xfaaaaaab, 0xfa2aaaab, 0xf9aaaaab, 0xf92aaaab, 
    0xf8aaaaab, 0xf82aaaab, 0xf7aaaaab, 0xf72aaaab, 0xf6aaaaab, 0xf62aaaab, 0xf5aaaaab, 0xf52aaaab, 
    0xf4aaaaab, 0xf42aaaab, 0xf3aaaaab, 0xf32aaaab, 0xf2aaaaab, 0xf22aaaab, 0xf1aaaaab, 0xf12aaaab, 
    0xf0aaaaab, 0xf02aaaab, 0xefaaaaab, 0xef2aaaab, 0xeeaaaaab, 0xee2aaaab, 0xedaaaaab, 0xed2aaaab, 
    0xecaaaaab, 0xec2aaaab, 0xebaaaaab, 0xeb2aaaab, 0xeaaaaaab, 0xea2aaaab, 0xe9aaaaab, 0xe92aaaab, 
    0xe8aaaaab, 0xe82aaaab, 0xe7aaaaab, 0xe72aaaab, 0xe6aaaaab, 0xe62aaaab, 0xe5aaaaab, 0xe52aaaab, 
    0xe4aaaaab, 0xe42aaaab, 0xe3aaaaab, 0xe32aaaab, 0xe2aaaaab, 0xe22aaaab, 0xe1aaaaab, 0xe12aaaab, 
    0xe0aaaaab, 0xe02aaaab, 0xdfaaaaab, 0xdf2aaaab, 0xdeaaaaab, 0xde2aaaab, 0xddaaaaab, 0xdd2aaaab, 
  };
  float32_t der_4_2[] = {
    0xdcaaaaab, 0xdc2aaaab, 0xdbaaaaab, 0xdb2aaaab, 0xdaaaaaab, 0xda2aaaab, 0xd9aaaaab, 0xd92aaaab, 
    0xd8aaaaab, 0xd82aaaab, 0xd7aaaaab, 0xd72aaaab, 0xd6aaaaab, 0xd62aaaab, 0xd5aaaaab, 0xd52aaaab, 
    0xd4aaaaab, 0xd42aaaab, 0xd3aaaaab, 0xd32aaaab, 0xd2aaaaab, 0xd22aaaab, 0xd1aaaaab, 0xd12aaaab, 
    0xd0aaaaab, 0xd02aaaab, 0xcfaaaaab, 0xcf2aaaab, 0xceaaaaab, 0xce2aaaab, 0xcdaaaaab, 0xcd2aaaab, 
    0xccaaaaab, 0xcc2aaaab, 0xcbaaaaab, 0xcb2aaaab, 0xcaaaaaab, 0xca2aaaab, 0xc9aaaaab, 0xc92aaaab, 
    0xc8aaaaab, 0xc82aaaab, 0xc7aaaaab, 0xc72aaaab, 0xc6aaaaab, 0xc62aaaab, 0xc5aaaaab, 0xc52aaaab, 
    0xc4aaaaab, 0xc42aaaab, 0xc3aaaaab, 0xc32aaaab, 0xc2aaaaaa, 0xc22aaaaa, 0xc1aaaaa8, 0xc12aaaa0, 
    0xc0aaaa80, 0xc02aaa00, 0xbfaaa800, 0xbf2aa000, 0xbeaa8000, 0xbe2a0000, 0xbda80000, 0xbd200000, 
  };
  float32_t der_4_3[] = {
    0xbc800000, 0x00000000, 0x3c800000, 0x3d200000, 0x3da80000, 0x3e2a0000, 0x3eaa8000, 0x3f2aa000, 
    0x3faaa800, 0x402aaa00, 0x40aaaa80, 0x412aaaa0, 0x41aaaaa8, 0x422aaaaa, 0x42aaaaaa, 0x432aaaab, 
    0x43aaaaab, 0x442aaaab, 0x44aaaaab, 0x452aaaab, 0x45aaaaab, 0x462aaaab, 0x46aaaaab, 0x472aaaab, 
    0x47aaaaab, 0x482aaaab, 0x48aaaaab, 0x492aaaab, 0x49aaaaab, 0x4a2aaaab, 0x4aaaaaab, 0x4b2aaaab, 
    0x4baaaaab, 0x4c2aaaab, 0x4caaaaab, 0x4d2aaaab, 0x4daaaaab, 0x4e2aaaab, 0x4eaaaaab, 0x4f2aaaab, 
    0x4faaaaab, 0x502aaaab, 0x50aaaaab, 0x512aaaab, 0x51aaaaab, 0x522aaaab, 0x52aaaaab, 0x532aaaab, 
    0x53aaaaab, 0x542aaaab, 0x54aaaaab, 0x552aaaab, 0x55aaaaab, 0x562aaaab, 0x56aaaaab, 0x572aaaab, 
    0x57aaaaab, 0x582aaaab, 0x58aaaaab, 0x592aaaab, 0x59aaaaab, 0x5a2aaaab, 0x5aaaaaab, 0x5b2aaaab, 
  };
  float32_t der_4_4[] = {
    0x5baaaaab, 0x5c2aaaab, 0x5caaaaab, 0x5d2aaaab, 0x5daaaaab, 0x5e2aaaab, 0x5eaaaaab, 0x5f2aaaab, 
    0x5faaaaab, 0x602aaaab, 0x60aaaaab, 0x612aaaab, 0x61aaaaab, 0x622aaaab, 0x62aaaaab, 0x632aaaab, 
    0x63aaaaab, 0x642aaaab, 0x64aaaaab, 0x652aaaab, 0x65aaaaab, 0x662aaaab, 0x66aaaaab, 0x672aaaab, 
    0x67aaaaab, 0x682aaaab, 0x68aaaaab, 0x692aaaab, 0x69aaaaab, 0x6a2aaaab, 0x6aaaaaab, 0x6b2aaaab, 
    0x6baaaaab, 0x6c2aaaab, 0x6caaaaab, 0x6d2aaaab, 0x6daaaaab, 0x6e2aaaab, 0x6eaaaaab, 0x6f2aaaab, 
    0x6faaaaab, 0x702aaaab, 0x70aaaaab, 0x712aaaab, 0x71aaaaab, 0x722aaaab, 0x72aaaaab, 0x732aaaab, 
    0x73aaaaab, 0x742aaaab, 0x74aaaaab, 0x752aaaab, 0x75aaaaab, 0x762aaaab, 0x76aaaaab, 0x772aaaab, 
    0x77aaaaab, 0x782aaaab, 0x78aaaaab, 0x792aaaab, 0x79aaaaab, 0x7a2aaaab, 0x7aaaaaab, 0x7b2aaaab, 
  };
  float32_t der_4_5[] = {
    0x7baaaaab, 0x7c2aaaab, 0x7caaaaab,    
  };

  float32_t val_n, der_1, der_2, der_3, der_4, ln2_n;
  n_int = n_int + 129;
      
  if( n_int < 64 )
  {
    val_n = val_n_1[ n_int ];
    der_1 = der_1_1[ n_int ];
    der_2 = der_2_1[ n_int ];
    der_3 = der_3_1[ n_int ];
    der_4 = der_4_1[ n_int ];
    ln2_n = ln2_n_1[ n_int ];
  }
  else if( n_int < 128 )
  {
    val_n = val_n_2[ n_int - 64 ];
    der_1 = der_1_2[ n_int - 64 ];
    der_2 = der_2_2[ n_int - 64 ];
    der_3 = der_3_2[ n_int - 64 ];
    der_4 = der_4_2[ n_int - 64 ];
    ln2_n = ln2_n_2[ n_int - 64 ];
  }
  else if( n_int < 192 )
  {
    val_n = val_n_3[ n_int - 128 ];
    der_1 = der_1_3[ n_int - 128 ];
    der_2 = der_2_3[ n_int - 128 ];
    der_3 = der_3_3[ n_int - 128 ];
    der_4 = der_4_3[ n_int - 128 ];
    ln2_n = ln2_n_3[ n_int - 128 ];
  }
  else if( n_int < 256 )
  {
    val_n = val_n_4[ n_int - 192 ];
    der_1 = der_1_4[ n_int - 192 ];
    der_2 = der_2_4[ n_int - 192 ];
    der_3 = der_3_4[ n_int - 192 ];
    der_4 = der_4_4[ n_int - 192 ];
    ln2_n = ln2_n_4[ n_int - 192 ];
  }
  else
  {
    val_n = val_n_5[ n_int - 256 ];
    der_1 = der_1_5[ n_int - 256 ];
    der_2 = der_2_5[ n_int - 256 ];
    der_3 = der_3_5[ n_int - 256 ];
    der_4 = der_4_5[ n_int - 256 ];
    ln2_n = ln2_n_5[ n_int - 256 ];  
  }

  float32_t dx = f32_sub( a, ln2_n );

  //taylor series
  uZ.f = f32_add( val_n, f32_mul( dx,
        f32_add( der_1, f32_mul( dx,
        f32_add( der_2, f32_mul( dx, 
        f32_add( der_3, f32_mul( dx, 
        der_4 ) ) ) ) ) ) ) );

  uZ.f = f32_mul( uZ.f, two );

  return uZ.f;

}