#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"
#include <stdio.h>

bfloat16_t bf16_sinh( bfloat16_t a )
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

  //if expA = 0x1f, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xffc0 : 0x7fc0;//-NaN or NaN;
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0xff80 : 0x7f80;//-inf or inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( ( a.v & 0x7FFF ) >= 0x42b3 )
  {
    uiZ = signA ? 0xff80 : 0x7f80;//-inf or inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  /** when x=nln2, the sinh(x) and all levels of deratives are easy to compute, 
   * so use the taylor series when x close to x0=nln2.
   */ 
  float32_t ln2_val_, ln2_recip_;
  ln2_val_.v = 0x3f317218; // ln2=0.6931471805599453
  ln2_recip_.v = 0x3fb8aa3b; // 1/ln2=1.4426950408889634

  float32_t n = f32_roundToInt( f32_mul( bf16_to_f32( a ), ln2_recip_ ), softfloat_round_near_maxMag, false ); // n = round(x/ln2) -14~14
  int32_t n_int = f32_to_i32( n, softfloat_round_near_maxMag, false );
  float32_t x0 = f32_mul( n, ln2_val_ ); // x0 = n * ln2
  bfloat16_t dx = f32_to_bf16( f32_sub( bf16_to_f32( a ), x0 ) );


  //taylor series
  bfloat16_t val_n_1[] = { 
  0xff00, 0xfe80, 0xfe00, 0xfd80, 0xfd00, 0xfc80, 0xfc00, 0xfb80, 0xfb00, 0xfa80, 0xfa00, 0xf980, 0xf900, 0xf880, 0xf800, 0xf780, 
  0xf700, 0xf680, 0xf600, 0xf580, 0xf500, 0xf480, 0xf400, 0xf380, 0xf300, 0xf280, 0xf200, 0xf180, 0xf100, 0xf080, 0xf000, 0xef80, 
  0xef00, 0xee80, 0xee00, 0xed80, 0xed00, 0xec80, 0xec00, 0xeb80, 0xeb00, 0xea80, 0xea00, 0xe980, 0xe900, 0xe880, 0xe800, 0xe780, 
  0xe700, 0xe680, 0xe600, 0xe580, 0xe500, 0xe480, 0xe400, 0xe380, 0xe300, 0xe280, 0xe200, 0xe180, 0xe100, 0xe080, 0xe000, 0xdf80,
  };
  bfloat16_t val_n_2[] = {
  0xdf00, 0xde80, 0xde00, 0xdd80, 0xdd00, 0xdc80, 0xdc00, 0xdb80, 0xdb00, 0xda80, 0xda00, 0xd980, 0xd900, 0xd880, 0xd800, 0xd780, 
  0xd700, 0xd680, 0xd600, 0xd580, 0xd500, 0xd480, 0xd400, 0xd380, 0xd300, 0xd280, 0xd200, 0xd180, 0xd100, 0xd080, 0xd000, 0xcf80, 
  0xcf00, 0xce80, 0xce00, 0xcd80, 0xcd00, 0xcc80, 0xcc00, 0xcb80, 0xcb00, 0xca80, 0xca00, 0xc980, 0xc900, 0xc880, 0xc800, 0xc780, 
  0xc700, 0xc680, 0xc600, 0xc580, 0xc500, 0xc480, 0xc400, 0xc380, 0xc300, 0xc280, 0xc200, 0xc180, 0xc0ff, 0xc07c, 0xbff0, 0xbf40,
  };
  bfloat16_t val_n_3[] = { 0x0000 };
  bfloat16_t val_n_4[] = {
  0x3f40, 0x3ff0, 0x407c, 0x40ff, 0x4180, 0x4200, 0x4280, 0x4300, 0x4380, 0x4400, 0x4480, 0x4500, 0x4580, 0x4600, 0x4680, 0x4700, 
  0x4780, 0x4800, 0x4880, 0x4900, 0x4980, 0x4a00, 0x4a80, 0x4b00, 0x4b80, 0x4c00, 0x4c80, 0x4d00, 0x4d80, 0x4e00, 0x4e80, 0x4f00, 
  0x4f80, 0x5000, 0x5080, 0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380, 0x5400, 0x5480, 0x5500, 0x5580, 0x5600, 0x5680, 0x5700, 
  0x5780, 0x5800, 0x5880, 0x5900, 0x5980, 0x5a00, 0x5a80, 0x5b00, 0x5b80, 0x5c00, 0x5c80, 0x5d00, 0x5d80, 0x5e00, 0x5e80, 0x5f00, 
  };
  bfloat16_t val_n_5[] = {
  0x5f80, 0x6000, 0x6080, 0x6100, 0x6180, 0x6200, 0x6280, 0x6300, 0x6380, 0x6400, 0x6480, 0x6500, 0x6580, 0x6600, 0x6680, 0x6700, 
  0x6780, 0x6800, 0x6880, 0x6900, 0x6980, 0x6a00, 0x6a80, 0x6b00, 0x6b80, 0x6c00, 0x6c80, 0x6d00, 0x6d80, 0x6e00, 0x6e80, 0x6f00, 
  0x6f80, 0x7000, 0x7080, 0x7100, 0x7180, 0x7200, 0x7280, 0x7300, 0x7380, 0x7400, 0x7480, 0x7500, 0x7580, 0x7600, 0x7680, 0x7700, 
  0x7780, 0x7800, 0x7880, 0x7900, 0x7980, 0x7a00, 0x7a80, 0x7b00, 0x7b80, 0x7c00, 0x7c80, 0x7d00, 0x7d80, 0x7e00, 0x7e80, 0x7f00 
  };
  bfloat16_t der_1_1[] = { 
  0x7f00, 0x7e80, 0x7e00, 0x7d80, 0x7d00, 0x7c80, 0x7c00, 0x7b80, 0x7b00, 0x7a80, 0x7a00, 0x7980, 0x7900, 0x7880, 0x7800, 0x7780, 
  0x7700, 0x7680, 0x7600, 0x7580, 0x7500, 0x7480, 0x7400, 0x7380, 0x7300, 0x7280, 0x7200, 0x7180, 0x7100, 0x7080, 0x7000, 0x6f80, 
  0x6f00, 0x6e80, 0x6e00, 0x6d80, 0x6d00, 0x6c80, 0x6c00, 0x6b80, 0x6b00, 0x6a80, 0x6a00, 0x6980, 0x6900, 0x6880, 0x6800, 0x6780, 
  0x6700, 0x6680, 0x6600, 0x6580, 0x6500, 0x6480, 0x6400, 0x6380, 0x6300, 0x6280, 0x6200, 0x6180, 0x6100, 0x6080, 0x6000, 0x5f80, 
  };
  bfloat16_t der_1_2[] = {
  0x5f00, 0x5e80, 0x5e00, 0x5d80, 0x5d00, 0x5c80, 0x5c00, 0x5b80, 0x5b00, 0x5a80, 0x5a00, 0x5980, 0x5900, 0x5880, 0x5800, 0x5780, 
  0x5700, 0x5680, 0x5600, 0x5580, 0x5500, 0x5480, 0x5400, 0x5380, 0x5300, 0x5280, 0x5200, 0x5180, 0x5100, 0x5080, 0x5000, 0x4f80, 
  0x4f00, 0x4e80, 0x4e00, 0x4d80, 0x4d00, 0x4c80, 0x4c00, 0x4b80, 0x4b00, 0x4a80, 0x4a00, 0x4980, 0x4900, 0x4880, 0x4800, 0x4780, 
  0x4700, 0x4680, 0x4600, 0x4580, 0x4500, 0x4480, 0x4400, 0x4380, 0x4300, 0x4280, 0x4200, 0x4180, 0x4100, 0x4082, 0x4008, 0x3fa0, 
  };
  bfloat16_t der_1_3[] = { 0x3f80 }; 
  bfloat16_t der_1_4[] = { 
  0x3fa0, 0x4008, 0x4082, 0x4100, 0x4180, 0x4200, 0x4280, 0x4300, 0x4380, 0x4400, 0x4480, 0x4500, 0x4580, 0x4600, 0x4680, 0x4700, 
  0x4780, 0x4800, 0x4880, 0x4900, 0x4980, 0x4a00, 0x4a80, 0x4b00, 0x4b80, 0x4c00, 0x4c80, 0x4d00, 0x4d80, 0x4e00, 0x4e80, 0x4f00, 
  0x4f80, 0x5000, 0x5080, 0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380, 0x5400, 0x5480, 0x5500, 0x5580, 0x5600, 0x5680, 0x5700, 
  0x5780, 0x5800, 0x5880, 0x5900, 0x5980, 0x5a00, 0x5a80, 0x5b00, 0x5b80, 0x5c00, 0x5c80, 0x5d00, 0x5d80, 0x5e00, 0x5e80, 0x5f00, 
  };
  bfloat16_t der_1_5[] = {
  0x5f80, 0x6000, 0x6080, 0x6100, 0x6180, 0x6200, 0x6280, 0x6300, 0x6380, 0x6400, 0x6480, 0x6500, 0x6580, 0x6600, 0x6680, 0x6700, 
  0x6780, 0x6800, 0x6880, 0x6900, 0x6980, 0x6a00, 0x6a80, 0x6b00, 0x6b80, 0x6c00, 0x6c80, 0x6d00, 0x6d80, 0x6e00, 0x6e80, 0x6f00, 
  0x6f80, 0x7000, 0x7080, 0x7100, 0x7180, 0x7200, 0x7280, 0x7300, 0x7380, 0x7400, 0x7480, 0x7500, 0x7580, 0x7600, 0x7680, 0x7700, 
  0x7780, 0x7800, 0x7880, 0x7900, 0x7980, 0x7a00, 0x7a80, 0x7b00, 0x7b80, 0x7c00, 0x7c80, 0x7d00, 0x7d80, 0x7e00, 0x7e80, 0x7f00 
  };
  bfloat16_t der_2_1[] = { 
  0xfe80, 0xfe00, 0xfd80, 0xfd00, 0xfc80, 0xfc00, 0xfb80, 0xfb00, 0xfa80, 0xfa00, 0xf980, 0xf900, 0xf880, 0xf800, 0xf780, 0xf700,
  0xf680, 0xf600, 0xf580, 0xf500, 0xf480, 0xf400, 0xf380, 0xf300, 0xf280, 0xf200, 0xf180, 0xf100, 0xf080, 0xf000, 0xef80, 0xef00, 
  0xee80, 0xee00, 0xed80, 0xed00, 0xec80, 0xec00, 0xeb80, 0xeb00, 0xea80, 0xea00, 0xe980, 0xe900, 0xe880, 0xe800, 0xe780, 0xe700, 
  0xe680, 0xe600, 0xe580, 0xe500, 0xe480, 0xe400, 0xe380, 0xe300, 0xe280, 0xe200, 0xe180, 0xe100, 0xe080, 0xe000, 0xdf80, 0xdf00, 
  };
  bfloat16_t der_2_2[] = {
  0xde80, 0xde00, 0xdd80, 0xdd00, 0xdc80, 0xdc00, 0xdb80, 0xdb00, 0xda80, 0xda00, 0xd980, 0xd900, 0xd880, 0xd800, 0xd780, 0xd700, 
  0xd680, 0xd600, 0xd580, 0xd500, 0xd480, 0xd400, 0xd380, 0xd300, 0xd280, 0xd200, 0xd180, 0xd100, 0xd080, 0xd000, 0xcf80, 0xcf00, 
  0xce80, 0xce00, 0xcd80, 0xcd00, 0xcc80, 0xcc00, 0xcb80, 0xcb00, 0xca80, 0xca00, 0xc980, 0xc900, 0xc880, 0xc800, 0xc780, 0xc700, 
  0xc680, 0xc600, 0xc580, 0xc500, 0xc480, 0xc400, 0xc380, 0xc300, 0xc280, 0xc200, 0xc180, 0xc100, 0xc07f, 0xbffc, 0xbf70, 0xbec0, 
  };
  bfloat16_t der_2_3[] = { 0x0000 };
  bfloat16_t der_2_4[] = {
  0x3ec0, 0x3f70, 0x3ffc, 0x407f, 0x4100, 0x4180, 0x4200, 0x4280, 0x4300, 0x4380, 0x4400, 0x4480, 0x4500, 0x4580, 0x4600, 0x4680, 
  0x4700, 0x4780, 0x4800, 0x4880, 0x4900, 0x4980, 0x4a00, 0x4a80, 0x4b00, 0x4b80, 0x4c00, 0x4c80, 0x4d00, 0x4d80, 0x4e00, 0x4e80, 
  0x4f00, 0x4f80, 0x5000, 0x5080, 0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380, 0x5400, 0x5480, 0x5500, 0x5580, 0x5600, 0x5680, 
  0x5700, 0x5780, 0x5800, 0x5880, 0x5900, 0x5980, 0x5a00, 0x5a80, 0x5b00, 0x5b80, 0x5c00, 0x5c80, 0x5d00, 0x5d80, 0x5e00, 0x5e80, 
};
  bfloat16_t der_2_5[] = {
  0x5f00, 0x5f80, 0x6000, 0x6080, 0x6100, 0x6180, 0x6200, 0x6280, 0x6300, 0x6380, 0x6400, 0x6480, 0x6500, 0x6580, 0x6600, 0x6680, 
  0x6700, 0x6780, 0x6800, 0x6880, 0x6900, 0x6980, 0x6a00, 0x6a80, 0x6b00, 0x6b80, 0x6c00, 0x6c80, 0x6d00, 0x6d80, 0x6e00, 0x6e80, 
  0x6f00, 0x6f80, 0x7000, 0x7080, 0x7100, 0x7180, 0x7200, 0x7280, 0x7300, 0x7380, 0x7400, 0x7480, 0x7500, 0x7580, 0x7600, 0x7680, 
  0x7700, 0x7780, 0x7800, 0x7880, 0x7900, 0x7980, 0x7a00, 0x7a80, 0x7b00, 0x7b80, 0x7c00, 0x7c80, 0x7d00, 0x7d80, 0x7e00, 0x7e80
  };
  bfloat16_t der_3_1[] = { 
  0x7dab, 0x7d2b, 0x7cab, 0x7c2b, 0x7bab, 0x7b2b, 0x7aab, 0x7a2b, 0x79ab, 0x792b, 0x78ab, 0x782b, 0x77ab, 0x772b, 0x76ab, 0x762b, 
  0x75ab, 0x752b, 0x74ab, 0x742b, 0x73ab, 0x732b, 0x72ab, 0x722b, 0x71ab, 0x712b, 0x70ab, 0x702b, 0x6fab, 0x6f2b, 0x6eab, 0x6e2b, 
  0x6dab, 0x6d2b, 0x6cab, 0x6c2b, 0x6bab, 0x6b2b, 0x6aab, 0x6a2b, 0x69ab, 0x692b, 0x68ab, 0x682b, 0x67ab, 0x672b, 0x66ab, 0x662b, 
  0x65ab, 0x652b, 0x64ab, 0x642b, 0x63ab, 0x632b, 0x62ab, 0x622b, 0x61ab, 0x612b, 0x60ab, 0x602b, 0x5fab, 0x5f2b, 0x5eab, 0x5e2b, 
  };
  bfloat16_t der_3_2[] = {
  0x5dab, 0x5d2b, 0x5cab, 0x5c2b, 0x5bab, 0x5b2b, 0x5aab, 0x5a2b, 0x59ab, 0x592b, 0x58ab, 0x582b, 0x57ab, 0x572b, 0x56ab, 0x562b, 
  0x55ab, 0x552b, 0x54ab, 0x542b, 0x53ab, 0x532b, 0x52ab, 0x522b, 0x51ab, 0x512b, 0x50ab, 0x502b, 0x4fab, 0x4f2b, 0x4eab, 0x4e2b, 
  0x4dab, 0x4d2b, 0x4cab, 0x4c2b, 0x4bab, 0x4b2b, 0x4aab, 0x4a2b, 0x49ab, 0x492b, 0x48ab, 0x482b, 0x47ab, 0x472b, 0x46ab, 0x462b, 
  0x45ab, 0x452b, 0x44ab, 0x442b, 0x43ab, 0x432b, 0x42ab, 0x422b, 0x41ab, 0x412b, 0x40ab, 0x402b, 0x3fab, 0x3f2d, 0x3eb5, 0x3e55, 
};
  bfloat16_t der_3_3[] = { 0x3e2b };
  bfloat16_t der_3_4[] = { 
  0x3e55, 0x3eb5, 0x3f2d, 0x3fab, 0x402b, 0x40ab, 0x412b, 0x41ab, 0x422b, 0x42ab, 0x432b, 0x43ab, 0x442b, 0x44ab, 0x452b, 0x45ab, 
  0x462b, 0x46ab, 0x472b, 0x47ab, 0x482b, 0x48ab, 0x492b, 0x49ab, 0x4a2b, 0x4aab, 0x4b2b, 0x4bab, 0x4c2b, 0x4cab, 0x4d2b, 0x4dab, 
  0x4e2b, 0x4eab, 0x4f2b, 0x4fab, 0x502b, 0x50ab, 0x512b, 0x51ab, 0x522b, 0x52ab, 0x532b, 0x53ab, 0x542b, 0x54ab, 0x552b, 0x55ab, 
  0x562b, 0x56ab, 0x572b, 0x57ab, 0x582b, 0x58ab, 0x592b, 0x59ab, 0x5a2b, 0x5aab, 0x5b2b, 0x5bab, 0x5c2b, 0x5cab, 0x5d2b, 0x5dab, 
  };
  bfloat16_t der_3_5[] = {
  0x5e2b, 0x5eab, 0x5f2b, 0x5fab, 0x602b, 0x60ab, 0x612b, 0x61ab, 0x622b, 0x62ab, 0x632b, 0x63ab, 0x642b, 0x64ab, 0x652b, 0x65ab, 
  0x662b, 0x66ab, 0x672b, 0x67ab, 0x682b, 0x68ab, 0x692b, 0x69ab, 0x6a2b, 0x6aab, 0x6b2b, 0x6bab, 0x6c2b, 0x6cab, 0x6d2b, 0x6dab, 
  0x6e2b, 0x6eab, 0x6f2b, 0x6fab, 0x702b, 0x70ab, 0x712b, 0x71ab, 0x722b, 0x72ab, 0x732b, 0x73ab, 0x742b, 0x74ab, 0x752b, 0x75ab, 
  0x762b, 0x76ab, 0x772b, 0x77ab, 0x782b, 0x78ab, 0x792b, 0x79ab, 0x7a2b, 0x7aab, 0x7b2b, 0x7bab, 0x7c2b, 0x7cab, 0x7d2b, 0x7dab
  };

  bfloat16_t val_n, der_1, der_2, der_3;
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
  

  uZ.f = bf16_add( val_n, bf16_mul( dx,
        bf16_add( der_1, bf16_mul( dx,
        bf16_add( der_2, bf16_mul( dx, 
        der_3 ) ) ) ) ) );

  return uZ.f;
}