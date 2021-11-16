#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

float32_t f32_cosh( float32_t a )
{

  float32_t uP1;
  float32_t uP2;
  float32_t uP3;

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

  //if expA = 0xFf, handle the NaN and infinite number
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
    uiZ = 0x7f800000;//+inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( ( a.v & 0x7FFFFFFF ) >= 0x42b2d4fd )
  {
    uiZ = 0x7f800000;//+inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  //use taylor series at x0=nln2 to compute cosh(x), where x is close to x0
  //when x=nln2, its easy to compute cosh(x0) and all levels of derivative
  float32_t ln2_val, ln2_recip;
  ln2_val.v = 0x3f317218; // ln2=0.6931471805599453
  ln2_recip.v = 0x3fb8aa3b; // 1/ln2=1.4426950408889634

  float32_t n = f32_roundToInt( f32_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2)
  int32_t n_int = f32_to_i32( n, softfloat_round_near_maxMag, false );
  float32_t x0 = f32_mul( n, ln2_val ); // x0 = n * ln2
  float32_t dx = f32_sub( a, x0 );

  //taylor series
  float32_t val_n_1[] = { 
  0x7f000000, 0x7e800000, 0x7e000000, 0x7d800000, 0x7d000000, 0x7c800000, 0x7c000000, 0x7b800000, 
  0x7b000000, 0x7a800000, 0x7a000000, 0x79800000, 0x79000000, 0x78800000, 0x78000000, 0x77800000, 
  0x77000000, 0x76800000, 0x76000000, 0x75800000, 0x75000000, 0x74800000, 0x74000000, 0x73800000, 
  0x73000000, 0x72800000, 0x72000000, 0x71800000, 0x71000000, 0x70800000, 0x70000000, 0x6f800000, 
  0x6f000000, 0x6e800000, 0x6e000000, 0x6d800000, 0x6d000000, 0x6c800000, 0x6c000000, 0x6b800000, 
  0x6b000000, 0x6a800000, 0x6a000000, 0x69800000, 0x69000000, 0x68800000, 0x68000000, 0x67800000, 
  0x67000000, 0x66800000, 0x66000000, 0x65800000, 0x65000000, 0x64800000, 0x64000000, 0x63800000, 
  0x63000000, 0x62800000, 0x62000000, 0x61800000, 0x61000000, 0x60800000, 0x60000000, 0x5f800000, 
  };
  float32_t val_n_2[] = {
  0x5f000000, 0x5e800000, 0x5e000000, 0x5d800000, 0x5d000000, 0x5c800000, 0x5c000000, 0x5b800000, 
  0x5b000000, 0x5a800000, 0x5a000000, 0x59800000, 0x59000000, 0x58800000, 0x58000000, 0x57800000, 
  0x57000000, 0x56800000, 0x56000000, 0x55800000, 0x55000000, 0x54800000, 0x54000000, 0x53800000, 
  0x53000000, 0x52800000, 0x52000000, 0x51800000, 0x51000000, 0x50800000, 0x50000000, 0x4f800000, 
  0x4f000000, 0x4e800000, 0x4e000000, 0x4d800000, 0x4d000000, 0x4c800000, 0x4c000000, 0x4b800000, 
  0x4b000000, 0x4a800000, 0x4a000000, 0x49800000, 0x49000000, 0x48800000, 0x48000000, 0x47800000, 
  0x47000000, 0x46800000, 0x46000000, 0x45800000, 0x45000000, 0x44800002, 0x44000008, 0x43800020, 
  0x43000080, 0x42800200, 0x42000800, 0x41802000, 0x41008000, 0x40820000, 0x40080000, 0x3fa00000, 
  };
  float32_t val_n_3[] = { 0x3f800000 };
  float32_t val_n_4[] = { 
  0x3fa00000, 0x40080000, 0x40820000, 0x41008000, 0x41802000, 0x42000800, 0x42800200, 0x43000080, 
  0x43800020, 0x44000008, 0x44800002, 0x45000000, 0x45800000, 0x46000000, 0x46800000, 0x47000000, 
  0x47800000, 0x48000000, 0x48800000, 0x49000000, 0x49800000, 0x4a000000, 0x4a800000, 0x4b000000, 
  0x4b800000, 0x4c000000, 0x4c800000, 0x4d000000, 0x4d800000, 0x4e000000, 0x4e800000, 0x4f000000, 
  0x4f800000, 0x50000000, 0x50800000, 0x51000000, 0x51800000, 0x52000000, 0x52800000, 0x53000000, 
  0x53800000, 0x54000000, 0x54800000, 0x55000000, 0x55800000, 0x56000000, 0x56800000, 0x57000000, 
  0x57800000, 0x58000000, 0x58800000, 0x59000000, 0x59800000, 0x5a000000, 0x5a800000, 0x5b000000, 
  0x5b800000, 0x5c000000, 0x5c800000, 0x5d000000, 0x5d800000, 0x5e000000, 0x5e800000, 0x5f000000, 
  };
  float32_t val_n_5[] = {
  0x5f800000, 0x60000000, 0x60800000, 0x61000000, 0x61800000, 0x62000000, 0x62800000, 0x63000000, 
  0x63800000, 0x64000000, 0x64800000, 0x65000000, 0x65800000, 0x66000000, 0x66800000, 0x67000000, 
  0x67800000, 0x68000000, 0x68800000, 0x69000000, 0x69800000, 0x6a000000, 0x6a800000, 0x6b000000, 
  0x6b800000, 0x6c000000, 0x6c800000, 0x6d000000, 0x6d800000, 0x6e000000, 0x6e800000, 0x6f000000, 
  0x6f800000, 0x70000000, 0x70800000, 0x71000000, 0x71800000, 0x72000000, 0x72800000, 0x73000000, 
  0x73800000, 0x74000000, 0x74800000, 0x75000000, 0x75800000, 0x76000000, 0x76800000, 0x77000000, 
  0x77800000, 0x78000000, 0x78800000, 0x79000000, 0x79800000, 0x7a000000, 0x7a800000, 0x7b000000, 
  0x7b800000, 0x7c000000, 0x7c800000, 0x7d000000, 0x7d800000, 0x7e000000, 0x7e800000, 0x7f000000
  };
  float32_t der_1_1[] = { 
  0xff000000, 0xfe800000, 0xfe000000, 0xfd800000, 0xfd000000, 0xfc800000, 0xfc000000, 0xfb800000, 
  0xfb000000, 0xfa800000, 0xfa000000, 0xf9800000, 0xf9000000, 0xf8800000, 0xf8000000, 0xf7800000, 
  0xf7000000, 0xf6800000, 0xf6000000, 0xf5800000, 0xf5000000, 0xf4800000, 0xf4000000, 0xf3800000, 
  0xf3000000, 0xf2800000, 0xf2000000, 0xf1800000, 0xf1000000, 0xf0800000, 0xf0000000, 0xef800000, 
  0xef000000, 0xee800000, 0xee000000, 0xed800000, 0xed000000, 0xec800000, 0xec000000, 0xeb800000, 
  0xeb000000, 0xea800000, 0xea000000, 0xe9800000, 0xe9000000, 0xe8800000, 0xe8000000, 0xe7800000, 
  0xe7000000, 0xe6800000, 0xe6000000, 0xe5800000, 0xe5000000, 0xe4800000, 0xe4000000, 0xe3800000, 
  0xe3000000, 0xe2800000, 0xe2000000, 0xe1800000, 0xe1000000, 0xe0800000, 0xe0000000, 0xdf800000, 
  };
  float32_t der_1_2[] = { 
  0xdf000000, 0xde800000, 0xde000000, 0xdd800000, 0xdd000000, 0xdc800000, 0xdc000000, 0xdb800000, 
  0xdb000000, 0xda800000, 0xda000000, 0xd9800000, 0xd9000000, 0xd8800000, 0xd8000000, 0xd7800000, 
  0xd7000000, 0xd6800000, 0xd6000000, 0xd5800000, 0xd5000000, 0xd4800000, 0xd4000000, 0xd3800000, 
  0xd3000000, 0xd2800000, 0xd2000000, 0xd1800000, 0xd1000000, 0xd0800000, 0xd0000000, 0xcf800000, 
  0xcf000000, 0xce800000, 0xce000000, 0xcd800000, 0xcd000000, 0xcc800000, 0xcc000000, 0xcb800000, 
  0xcb000000, 0xca800000, 0xca000000, 0xc9800000, 0xc9000000, 0xc8800000, 0xc8000000, 0xc7800000, 
  0xc7000000, 0xc6800000, 0xc6000000, 0xc5800000, 0xc4ffffff, 0xc47ffffc, 0xc3fffff0, 0xc37fffc0, 
  0xc2ffff00, 0xc27ffc00, 0xc1fff000, 0xc17fc000, 0xc0ff0000, 0xc07c0000, 0xbff00000, 0xbf400000, 
  };
  float32_t der_1_3[] = { 0x00000000 };
  float32_t der_1_4[] = { 
  0x3f400000, 0x3ff00000, 0x407c0000, 0x40ff0000, 0x417fc000, 0x41fff000, 0x427ffc00, 0x42ffff00, 
  0x437fffc0, 0x43fffff0, 0x447ffffc, 0x44ffffff, 0x45800000, 0x46000000, 0x46800000, 0x47000000, 
  0x47800000, 0x48000000, 0x48800000, 0x49000000, 0x49800000, 0x4a000000, 0x4a800000, 0x4b000000, 
  0x4b800000, 0x4c000000, 0x4c800000, 0x4d000000, 0x4d800000, 0x4e000000, 0x4e800000, 0x4f000000, 
  0x4f800000, 0x50000000, 0x50800000, 0x51000000, 0x51800000, 0x52000000, 0x52800000, 0x53000000, 
  0x53800000, 0x54000000, 0x54800000, 0x55000000, 0x55800000, 0x56000000, 0x56800000, 0x57000000, 
  0x57800000, 0x58000000, 0x58800000, 0x59000000, 0x59800000, 0x5a000000, 0x5a800000, 0x5b000000, 
  0x5b800000, 0x5c000000, 0x5c800000, 0x5d000000, 0x5d800000, 0x5e000000, 0x5e800000, 0x5f000000, 
  };
  float32_t der_1_5[] = { 
  0x5f800000, 0x60000000, 0x60800000, 0x61000000, 0x61800000, 0x62000000, 0x62800000, 0x63000000, 
  0x63800000, 0x64000000, 0x64800000, 0x65000000, 0x65800000, 0x66000000, 0x66800000, 0x67000000, 
  0x67800000, 0x68000000, 0x68800000, 0x69000000, 0x69800000, 0x6a000000, 0x6a800000, 0x6b000000, 
  0x6b800000, 0x6c000000, 0x6c800000, 0x6d000000, 0x6d800000, 0x6e000000, 0x6e800000, 0x6f000000, 
  0x6f800000, 0x70000000, 0x70800000, 0x71000000, 0x71800000, 0x72000000, 0x72800000, 0x73000000, 
  0x73800000, 0x74000000, 0x74800000, 0x75000000, 0x75800000, 0x76000000, 0x76800000, 0x77000000, 
  0x77800000, 0x78000000, 0x78800000, 0x79000000, 0x79800000, 0x7a000000, 0x7a800000, 0x7b000000, 
  0x7b800000, 0x7c000000, 0x7c800000, 0x7d000000, 0x7d800000, 0x7e000000, 0x7e800000, 0x7f000000, 
  };
  float32_t der_2_1[] = { 
  0x7e800000, 0x7e000000, 0x7d800000, 0x7d000000, 0x7c800000, 0x7c000000, 0x7b800000, 0x7b000000, 
  0x7a800000, 0x7a000000, 0x79800000, 0x79000000, 0x78800000, 0x78000000, 0x77800000, 0x77000000, 
  0x76800000, 0x76000000, 0x75800000, 0x75000000, 0x74800000, 0x74000000, 0x73800000, 0x73000000, 
  0x72800000, 0x72000000, 0x71800000, 0x71000000, 0x70800000, 0x70000000, 0x6f800000, 0x6f000000, 
  0x6e800000, 0x6e000000, 0x6d800000, 0x6d000000, 0x6c800000, 0x6c000000, 0x6b800000, 0x6b000000, 
  0x6a800000, 0x6a000000, 0x69800000, 0x69000000, 0x68800000, 0x68000000, 0x67800000, 0x67000000, 
  0x66800000, 0x66000000, 0x65800000, 0x65000000, 0x64800000, 0x64000000, 0x63800000, 0x63000000, 
  0x62800000, 0x62000000, 0x61800000, 0x61000000, 0x60800000, 0x60000000, 0x5f800000, 0x5f000000, 
  };
  float32_t der_2_2[] = {
  0x5e800000, 0x5e000000, 0x5d800000, 0x5d000000, 0x5c800000, 0x5c000000, 0x5b800000, 0x5b000000, 
  0x5a800000, 0x5a000000, 0x59800000, 0x59000000, 0x58800000, 0x58000000, 0x57800000, 0x57000000, 
  0x56800000, 0x56000000, 0x55800000, 0x55000000, 0x54800000, 0x54000000, 0x53800000, 0x53000000, 
  0x52800000, 0x52000000, 0x51800000, 0x51000000, 0x50800000, 0x50000000, 0x4f800000, 0x4f000000, 
  0x4e800000, 0x4e000000, 0x4d800000, 0x4d000000, 0x4c800000, 0x4c000000, 0x4b800000, 0x4b000000, 
  0x4a800000, 0x4a000000, 0x49800000, 0x49000000, 0x48800000, 0x48000000, 0x47800000, 0x47000000, 
  0x46800000, 0x46000000, 0x45800000, 0x45000000, 0x44800000, 0x44000002, 0x43800008, 0x43000020, 
  0x42800080, 0x42000200, 0x41800800, 0x41002000, 0x40808000, 0x40020000, 0x3f880000, 0x3f200000, 
  };
  float32_t der_2_3[] = { 0x3f000000 };
  float32_t der_2_4[] = { 
  0x3f200000, 0x3f880000, 0x40020000, 0x40808000, 0x41002000, 0x41800800, 0x42000200, 0x42800080, 
  0x43000020, 0x43800008, 0x44000002, 0x44800000, 0x45000000, 0x45800000, 0x46000000, 0x46800000, 
  0x47000000, 0x47800000, 0x48000000, 0x48800000, 0x49000000, 0x49800000, 0x4a000000, 0x4a800000, 
  0x4b000000, 0x4b800000, 0x4c000000, 0x4c800000, 0x4d000000, 0x4d800000, 0x4e000000, 0x4e800000, 
  0x4f000000, 0x4f800000, 0x50000000, 0x50800000, 0x51000000, 0x51800000, 0x52000000, 0x52800000, 
  0x53000000, 0x53800000, 0x54000000, 0x54800000, 0x55000000, 0x55800000, 0x56000000, 0x56800000, 
  0x57000000, 0x57800000, 0x58000000, 0x58800000, 0x59000000, 0x59800000, 0x5a000000, 0x5a800000, 
  0x5b000000, 0x5b800000, 0x5c000000, 0x5c800000, 0x5d000000, 0x5d800000, 0x5e000000, 0x5e800000, 
  };
  float32_t der_2_5[] = {
  0x5f000000, 0x5f800000, 0x60000000, 0x60800000, 0x61000000, 0x61800000, 0x62000000, 0x62800000, 
  0x63000000, 0x63800000, 0x64000000, 0x64800000, 0x65000000, 0x65800000, 0x66000000, 0x66800000, 
  0x67000000, 0x67800000, 0x68000000, 0x68800000, 0x69000000, 0x69800000, 0x6a000000, 0x6a800000, 
  0x6b000000, 0x6b800000, 0x6c000000, 0x6c800000, 0x6d000000, 0x6d800000, 0x6e000000, 0x6e800000, 
  0x6f000000, 0x6f800000, 0x70000000, 0x70800000, 0x71000000, 0x71800000, 0x72000000, 0x72800000, 
  0x73000000, 0x73800000, 0x74000000, 0x74800000, 0x75000000, 0x75800000, 0x76000000, 0x76800000, 
  0x77000000, 0x77800000, 0x78000000, 0x78800000, 0x79000000, 0x79800000, 0x7a000000, 0x7a800000, 
  0x7b000000, 0x7b800000, 0x7c000000, 0x7c800000, 0x7d000000, 0x7d800000, 0x7e000000, 0x7e800000, 
  };
  float32_t der_3_1[] = { 
  0xfdaaaaab, 0xfd2aaaab, 0xfcaaaaab, 0xfc2aaaab, 0xfbaaaaab, 0xfb2aaaab, 0xfaaaaaab, 0xfa2aaaab, 
  0xf9aaaaab, 0xf92aaaab, 0xf8aaaaab, 0xf82aaaab, 0xf7aaaaab, 0xf72aaaab, 0xf6aaaaab, 0xf62aaaab, 
  0xf5aaaaab, 0xf52aaaab, 0xf4aaaaab, 0xf42aaaab, 0xf3aaaaab, 0xf32aaaab, 0xf2aaaaab, 0xf22aaaab, 
  0xf1aaaaab, 0xf12aaaab, 0xf0aaaaab, 0xf02aaaab, 0xefaaaaab, 0xef2aaaab, 0xeeaaaaab, 0xee2aaaab, 
  0xedaaaaab, 0xed2aaaab, 0xecaaaaab, 0xec2aaaab, 0xebaaaaab, 0xeb2aaaab, 0xeaaaaaab, 0xea2aaaab, 
  0xe9aaaaab, 0xe92aaaab, 0xe8aaaaab, 0xe82aaaab, 0xe7aaaaab, 0xe72aaaab, 0xe6aaaaab, 0xe62aaaab, 
  0xe5aaaaab, 0xe52aaaab, 0xe4aaaaab, 0xe42aaaab, 0xe3aaaaab, 0xe32aaaab, 0xe2aaaaab, 0xe22aaaab, 
  0xe1aaaaab, 0xe12aaaab, 0xe0aaaaab, 0xe02aaaab, 0xdfaaaaab, 0xdf2aaaab, 0xdeaaaaab, 0xde2aaaab, 
  };
  float32_t der_3_2[] = { 
  0xddaaaaab, 0xdd2aaaab, 0xdcaaaaab, 0xdc2aaaab, 0xdbaaaaab, 0xdb2aaaab, 0xdaaaaaab, 0xda2aaaab, 
  0xd9aaaaab, 0xd92aaaab, 0xd8aaaaab, 0xd82aaaab, 0xd7aaaaab, 0xd72aaaab, 0xd6aaaaab, 0xd62aaaab, 
  0xd5aaaaab, 0xd52aaaab, 0xd4aaaaab, 0xd42aaaab, 0xd3aaaaab, 0xd32aaaab, 0xd2aaaaab, 0xd22aaaab, 
  0xd1aaaaab, 0xd12aaaab, 0xd0aaaaab, 0xd02aaaab, 0xcfaaaaab, 0xcf2aaaab, 0xceaaaaab, 0xce2aaaab, 
  0xcdaaaaab, 0xcd2aaaab, 0xccaaaaab, 0xcc2aaaab, 0xcbaaaaab, 0xcb2aaaab, 0xcaaaaaab, 0xca2aaaab, 
  0xc9aaaaab, 0xc92aaaab, 0xc8aaaaab, 0xc82aaaab, 0xc7aaaaab, 0xc72aaaab, 0xc6aaaaab, 0xc62aaaab, 
  0xc5aaaaab, 0xc52aaaab, 0xc4aaaaab, 0xc42aaaaa, 0xc3aaaaaa, 0xc32aaaa8, 0xc2aaaaa0, 0xc22aaa80, 
  0xc1aaaa00, 0xc12aa800, 0xc0aaa000, 0xc02a8000, 0xbfaa0000, 0xbf280000, 0xbea00000, 0xbe000000, 
  };
  float32_t der_3_3[] = { 0x00000000 };
  float32_t der_3_4[] = {   
  0x3e000000, 0x3ea00000, 0x3f280000, 0x3faa0000, 0x402a8000, 0x40aaa000, 0x412aa800, 0x41aaaa00, 
  0x422aaa80, 0x42aaaaa0, 0x432aaaa8, 0x43aaaaaa, 0x442aaaaa, 0x44aaaaab, 0x452aaaab, 0x45aaaaab, 
  0x462aaaab, 0x46aaaaab, 0x472aaaab, 0x47aaaaab, 0x482aaaab, 0x48aaaaab, 0x492aaaab, 0x49aaaaab, 
  0x4a2aaaab, 0x4aaaaaab, 0x4b2aaaab, 0x4baaaaab, 0x4c2aaaab, 0x4caaaaab, 0x4d2aaaab, 0x4daaaaab, 
  0x4e2aaaab, 0x4eaaaaab, 0x4f2aaaab, 0x4faaaaab, 0x502aaaab, 0x50aaaaab, 0x512aaaab, 0x51aaaaab, 
  0x522aaaab, 0x52aaaaab, 0x532aaaab, 0x53aaaaab, 0x542aaaab, 0x54aaaaab, 0x552aaaab, 0x55aaaaab, 
  0x562aaaab, 0x56aaaaab, 0x572aaaab, 0x57aaaaab, 0x582aaaab, 0x58aaaaab, 0x592aaaab, 0x59aaaaab, 
  0x5a2aaaab, 0x5aaaaaab, 0x5b2aaaab, 0x5baaaaab, 0x5c2aaaab, 0x5caaaaab, 0x5d2aaaab, 0x5daaaaab, 
  };
  float32_t der_3_5[] = { 
  0x5e2aaaab, 0x5eaaaaab, 0x5f2aaaab, 0x5faaaaab, 0x602aaaab, 0x60aaaaab, 0x612aaaab, 0x61aaaaab, 
  0x622aaaab, 0x62aaaaab, 0x632aaaab, 0x63aaaaab, 0x642aaaab, 0x64aaaaab, 0x652aaaab, 0x65aaaaab, 
  0x662aaaab, 0x66aaaaab, 0x672aaaab, 0x67aaaaab, 0x682aaaab, 0x68aaaaab, 0x692aaaab, 0x69aaaaab, 
  0x6a2aaaab, 0x6aaaaaab, 0x6b2aaaab, 0x6baaaaab, 0x6c2aaaab, 0x6caaaaab, 0x6d2aaaab, 0x6daaaaab, 
  0x6e2aaaab, 0x6eaaaaab, 0x6f2aaaab, 0x6faaaaab, 0x702aaaab, 0x70aaaaab, 0x712aaaab, 0x71aaaaab, 
  0x722aaaab, 0x72aaaaab, 0x732aaaab, 0x73aaaaab, 0x742aaaab, 0x74aaaaab, 0x752aaaab, 0x75aaaaab, 
  0x762aaaab, 0x76aaaaab, 0x772aaaab, 0x77aaaaab, 0x782aaaab, 0x78aaaaab, 0x792aaaab, 0x79aaaaab, 
  0x7a2aaaab, 0x7aaaaaab, 0x7b2aaaab, 0x7baaaaab, 0x7c2aaaab, 0x7caaaaab, 0x7d2aaaab, 0x7daaaaab, 
  };

  float32_t val_n, der_1, der_2, der_3;
  if ( n_int < - 128 || n_int > 128 )
  {//taylor coefficients can't expressed in f32, 
   //so when n_int=-129, use exp(-(-128ln2+dx)) to compute the result
   //   when n_int= 129, use exp(128ln2+dx) to compute the result
    val_n.v = 0x3f800000 ;
    der_1.v = 0x3f800000 ;
    der_2.v = 0x3f000000 ;
    der_3.v = 0x3e2aaaab ;
    if( n_int < -128 )
    {
      dx.v = ( (uint32_t)(1) << 31 ) + dx.v;
    }
  }
  else
  {
  
    if( n_int >= -128 && n_int <= -65 )
    {
      val_n = val_n_1[ n_int + 128 ];
      der_1 = der_1_1[ n_int + 128 ];
      der_2 = der_2_1[ n_int + 128 ];
      der_3 = der_3_1[ n_int + 128 ];
    }
    else if( n_int <0 )
    {
      val_n = val_n_2[ n_int + 64 ];
      der_1 = der_1_2[ n_int + 64 ];
      der_2 = der_2_2[ n_int + 64 ];
      der_3 = der_3_2[ n_int + 64 ];
    }
    else if ( 0 == n_int )
    {
      val_n = val_n_3[ 0 ];
      der_1 = der_1_3[ 0 ];
      der_2 = der_2_3[ 0 ];
      der_3 = der_3_3[ 0 ];
    }
    else if( n_int < 65 )
    {
      val_n = val_n_4[ n_int - 1 ];
      der_1 = der_1_4[ n_int - 1 ];
      der_2 = der_2_4[ n_int - 1 ];
      der_3 = der_3_4[ n_int - 1 ];
    }
    else
    {
      val_n = val_n_5[ n_int - 65 ];
      der_1 = der_1_5[ n_int - 65 ];
      der_2 = der_2_5[ n_int - 65 ];
      der_3 = der_3_5[ n_int - 65 ];    
    }
  }

  uZ.f = f32_add( val_n, f32_mul( dx,
        f32_add( der_1, f32_mul( dx,
        f32_add( der_2, f32_mul( dx, 
        der_3 ) ) ) ) ) );
  
  if ( n_int < - 128 || n_int > 128 )
  {
    if( 0x42b2d4fc == ( a.v & 0x7FFFFFFF) )
    {
      uiZ = 0x7f7fffec;
      uZ.ui = uiZ;
    }
    else
    {
      uZ.ui = ((uint32_t) (128)<<23) + uZ.ui; 
    }
  }

  return uZ.f;

}