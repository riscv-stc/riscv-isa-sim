#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

// bfloat16_t bf16_exp( bfloat16_t a )
// {

//   bfloat16_t ln2 = { 0x3f31 } ; //0.69314718055994530941723212145818
//   bfloat16_t one = { 0x3f80 } ; //1
//   bfloat16_t uP3 = { 0x3e76 }; //ln2^2/2
//   bfloat16_t uP4 = { 0x3d63 }; //ln2^3/6

//   bfloat16_t  f;

//   uint_fast16_t uiA;
//   bool signA;
//   int_fast16_t expA;
//   uint_fast16_t sigA;
//   union ui16_bf16 uA;

//   uint_fast16_t uiZ;
//   union ui16_bf16 uZ;

//   //get the signal\exp\fraction bits from input float number a
//   uA.f = a;
//   uiA = uA.ui;
//   signA = signBF16UI( uiA );
//   expA = expBF16UI( uiA );
//   sigA = fracBF16UI( uiA );

//   //if expA = 0xFF, handle the NaN and infinite number
//   if( 0xFF == expA )
//   {
//     //if NaN input
//     if( sigA )
//     {
//       uiZ = signA ? 0xffc0 : 0x7fc0;//-NaN or NaN
//       uZ.ui = uiZ;
//       return uZ.f;
//     }
//     //infinite number input
//     uiZ = signA ? 0x0 : uiA; // 0 or +inf
//     uZ.ui = uiZ;
//     return uZ.f;
//   }

//   if( a.v > 0xc2b9 ) //a<-92.5
//   {
//     uZ.ui = 0;
//     return uZ.f;
//   }
//   //when a > 88.5, the result will be +inf
//   if( ( a.v > 0x42b1 ) && ( a.v < 0x8000 ) )
//   {
//     uZ.ui = 0x7f80; //+inf
//     return uZ.f;
//   }


//   //compute e^a as 2^(a/ln(2))=2^(z+f) which |f|<1
//   float32_t recip_ln2 = { 0x3fb8aa3b };
//   float32_t x = f32_mul( bf16_to_f32( a ), recip_ln2 );
//   float32_t z = f32_roundToInt( x, softfloat_round_near_maxMag, false );
//   f = f32_to_bf16( f32_sub( x, z ) );

//   //compute 2^f using taylor series
//   f = bf16_add( one, bf16_mul( f, 
//   bf16_add( ln2, bf16_mul( f, 
//   bf16_add( uP3, bf16_mul( f, uP4 ) ) ) ) ) );

//   //compute the exp bits of 2^(z+f)
//   int_fast16_t expz = f32_to_i32( z, softfloat_round_near_maxMag, false ) + ( ( ( int_fast16_t )( f.v & 0x7f80 ) ) >> 7 );
//   if( expz <= -7 )
//   {
//     uZ.ui = 0;
//     return uZ.f;
//   }
//   else if( expz < 1 )
//   {//result is too small
//     uZ.ui = ( ( f.v & 0x7F ) + 0x80 ) >> ( -expz + 1 );
//     return uZ.f;
//   }
//   else if( expz > 254 )
//   {//result is too big
//     uZ.ui = 0x7f80;//+inf
//     return uZ.f;
//   }
//   else
//   { //compute 2^(z+f)
//     uZ.ui = ( f.v & 0x807F ) +  ( expz << 7 );
//     return uZ.f;
//   }

// }

bfloat16_t bf16_exp( bfloat16_t a )
{

  bfloat16_t ln2 = { 0x3f31 } ; //0.69314718055994530941723212145818
  bfloat16_t one = { 0x3f80 } ; //1
  bfloat16_t uP3 = { 0x3e76 }; //ln2^2/2
  bfloat16_t uP4 = { 0x3d63 }; //ln2^3/6

  bfloat16_t  f;

  uint_fast16_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast16_t sigA;
  union ui16_bf16 uA;

  uint_fast16_t uiZ;
  union ui16_bf16 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signBF16UI( uiA );
  expA = expBF16UI( uiA );
  sigA = fracBF16UI( uiA );

  //if expA = 0xFF, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xffc0 : 0x7fc0;//-NaN or NaN
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0x0 : uiA; // 0 or +inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( a.v > 0xc2b9 ) //a<-92.5
  {
    uZ.ui = 0;
    return uZ.f;
  }
  //when a > 88.5, the result will be +inf
  if( ( a.v > 0x42b1 ) && ( a.v < 0x8000 ) )
  {
    uZ.ui = 0x7f80; //+inf
    return uZ.f;
  }

  if( 0x42b1 == a.v )
  {
    uZ.ui = 0x7f4d;
    return uZ.f;
  }

  if( a.v >= 0xc2ae && a.v <= 0xc2b9 )
  {
    uint8_t index = a.v - 0xc2ae;
    uint16_t makeup[] = { 0xb3, 0x6d, 0x42, 0x28, 0x18, 0xf, 0x9, 0x5, 0x3, 0x2, 0x1, 0x1 };
    uZ.ui = makeup[ index ];
    return uZ.f;
  }

  /** when x=nln2, the tanh(x) and all levels of deratives are easy to compute, 
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
    0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380, 0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780, 0x0800, 0x0880, 
    0x0900, 0x0980, 0x0a00, 0x0a80, 0x0b00, 0x0b80, 0x0c00, 0x0c80, 0x0d00, 0x0d80, 0x0e00, 0x0e80, 0x0f00, 0x0f80, 0x1000, 0x1080, 
    0x1100, 0x1180, 0x1200, 0x1280, 0x1300, 0x1380, 0x1400, 0x1480, 0x1500, 0x1580, 0x1600, 0x1680, 0x1700, 0x1780, 0x1800, 0x1880, 
    0x1900, 0x1980, 0x1a00, 0x1a80, 0x1b00, 0x1b80, 0x1c00, 0x1c80, 0x1d00, 0x1d80, 0x1e00, 0x1e80, 0x1f00, 0x1f80, 0x2000, 0x2080};
  bfloat16_t val_n_2[] = {  
    0x2100, 0x2180, 0x2200, 0x2280, 0x2300, 0x2380, 0x2400, 0x2480, 0x2500, 0x2580, 0x2600, 0x2680, 0x2700, 0x2780, 0x2800, 0x2880, 
    0x2900, 0x2980, 0x2a00, 0x2a80, 0x2b00, 0x2b80, 0x2c00, 0x2c80, 0x2d00, 0x2d80, 0x2e00, 0x2e80, 0x2f00, 0x2f80, 0x3000, 0x3080, 
    0x3100, 0x3180, 0x3200, 0x3280, 0x3300, 0x3380, 0x3400, 0x3480, 0x3500, 0x3580, 0x3600, 0x3680, 0x3700, 0x3780, 0x3800, 0x3880, 
    0x3900, 0x3980, 0x3a00, 0x3a80, 0x3b00, 0x3b80, 0x3c00, 0x3c80, 0x3d00, 0x3d80, 0x3e00, 0x3e80, 0x3f00, 0x3f80, 0x4000, 0x4080};
  bfloat16_t val_n_3[] = {  
    0x4100, 0x4180, 0x4200, 0x4280, 0x4300, 0x4380, 0x4400, 0x4480, 0x4500, 0x4580, 0x4600, 0x4680, 0x4700, 0x4780, 0x4800, 0x4880, 
    0x4900, 0x4980, 0x4a00, 0x4a80, 0x4b00, 0x4b80, 0x4c00, 0x4c80, 0x4d00, 0x4d80, 0x4e00, 0x4e80, 0x4f00, 0x4f80, 0x5000, 0x5080, 
    0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380, 0x5400, 0x5480, 0x5500, 0x5580, 0x5600, 0x5680, 0x5700, 0x5780, 0x5800, 0x5880, 
    0x5900, 0x5980, 0x5a00, 0x5a80, 0x5b00, 0x5b80, 0x5c00, 0x5c80, 0x5d00, 0x5d80, 0x5e00, 0x5e80, 0x5f00, 0x5f80, 0x6000, 0x6080}; 
  bfloat16_t val_n_4[] = {
    0x6100, 0x6180, 0x6200, 0x6280, 0x6300, 0x6380, 0x6400, 0x6480, 0x6500, 0x6580, 0x6600, 0x6680, 0x6700, 0x6780, 0x6800, 0x6880, 
    0x6900, 0x6980, 0x6a00, 0x6a80, 0x6b00, 0x6b80, 0x6c00, 0x6c80, 0x6d00, 0x6d80, 0x6e00, 0x6e80, 0x6f00, 0x6f80, 0x7000, 0x7080, 
    0x7100, 0x7180, 0x7200, 0x7280, 0x7300, 0x7380, 0x7400, 0x7480, 0x7500, 0x7580, 0x7600, 0x7680, 0x7700, 0x7780, 0x7800, 0x7880, 
    0x7900, 0x7980, 0x7a00, 0x7a80, 0x7b00, 0x7b80, 0x7c00, 0x7c80, 0x7d00, 0x7d80, 0x7e00, 0x7e80, 0x7f00 };
  bfloat16_t der_1_1[] = { 
    0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380, 0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780, 0x0800, 0x0880, 
    0x0900, 0x0980, 0x0a00, 0x0a80, 0x0b00, 0x0b80, 0x0c00, 0x0c80, 0x0d00, 0x0d80, 0x0e00, 0x0e80, 0x0f00, 0x0f80, 0x1000, 0x1080, 
    0x1100, 0x1180, 0x1200, 0x1280, 0x1300, 0x1380, 0x1400, 0x1480, 0x1500, 0x1580, 0x1600, 0x1680, 0x1700, 0x1780, 0x1800, 0x1880, 
    0x1900, 0x1980, 0x1a00, 0x1a80, 0x1b00, 0x1b80, 0x1c00, 0x1c80, 0x1d00, 0x1d80, 0x1e00, 0x1e80, 0x1f00, 0x1f80, 0x2000, 0x2080};
  bfloat16_t der_1_2[] = {  
    0x2100, 0x2180, 0x2200, 0x2280, 0x2300, 0x2380, 0x2400, 0x2480, 0x2500, 0x2580, 0x2600, 0x2680, 0x2700, 0x2780, 0x2800, 0x2880, 
    0x2900, 0x2980, 0x2a00, 0x2a80, 0x2b00, 0x2b80, 0x2c00, 0x2c80, 0x2d00, 0x2d80, 0x2e00, 0x2e80, 0x2f00, 0x2f80, 0x3000, 0x3080, 
    0x3100, 0x3180, 0x3200, 0x3280, 0x3300, 0x3380, 0x3400, 0x3480, 0x3500, 0x3580, 0x3600, 0x3680, 0x3700, 0x3780, 0x3800, 0x3880, 
    0x3900, 0x3980, 0x3a00, 0x3a80, 0x3b00, 0x3b80, 0x3c00, 0x3c80, 0x3d00, 0x3d80, 0x3e00, 0x3e80, 0x3f00, 0x3f80, 0x4000, 0x4080};
  bfloat16_t der_1_3[] = {  
    0x4100, 0x4180, 0x4200, 0x4280, 0x4300, 0x4380, 0x4400, 0x4480, 0x4500, 0x4580, 0x4600, 0x4680, 0x4700, 0x4780, 0x4800, 0x4880, 
    0x4900, 0x4980, 0x4a00, 0x4a80, 0x4b00, 0x4b80, 0x4c00, 0x4c80, 0x4d00, 0x4d80, 0x4e00, 0x4e80, 0x4f00, 0x4f80, 0x5000, 0x5080, 
    0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380, 0x5400, 0x5480, 0x5500, 0x5580, 0x5600, 0x5680, 0x5700, 0x5780, 0x5800, 0x5880, 
    0x5900, 0x5980, 0x5a00, 0x5a80, 0x5b00, 0x5b80, 0x5c00, 0x5c80, 0x5d00, 0x5d80, 0x5e00, 0x5e80, 0x5f00, 0x5f80, 0x6000, 0x6080}; 
  bfloat16_t der_1_4[] = {  
    0x6100, 0x6180, 0x6200, 0x6280, 0x6300, 0x6380, 0x6400, 0x6480, 0x6500, 0x6580, 0x6600, 0x6680, 0x6700, 0x6780, 0x6800, 0x6880, 
    0x6900, 0x6980, 0x6a00, 0x6a80, 0x6b00, 0x6b80, 0x6c00, 0x6c80, 0x6d00, 0x6d80, 0x6e00, 0x6e80, 0x6f00, 0x6f80, 0x7000, 0x7080, 
    0x7100, 0x7180, 0x7200, 0x7280, 0x7300, 0x7380, 0x7400, 0x7480, 0x7500, 0x7580, 0x7600, 0x7680, 0x7700, 0x7780, 0x7800, 0x7880, 
    0x7900, 0x7980, 0x7a00, 0x7a80, 0x7b00, 0x7b80, 0x7c00, 0x7c80, 0x7d00, 0x7d80, 0x7e00, 0x7e80, 0x7f00 };
  bfloat16_t der_2_1[] = { 
    0x0080, 0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380, 0x0400, 0x0480, 0x0500, 0x0580, 0x0600, 0x0680, 0x0700, 0x0780, 0x0800, 
    0x0880, 0x0900, 0x0980, 0x0a00, 0x0a80, 0x0b00, 0x0b80, 0x0c00, 0x0c80, 0x0d00, 0x0d80, 0x0e00, 0x0e80, 0x0f00, 0x0f80, 0x1000, 
    0x1080, 0x1100, 0x1180, 0x1200, 0x1280, 0x1300, 0x1380, 0x1400, 0x1480, 0x1500, 0x1580, 0x1600, 0x1680, 0x1700, 0x1780, 0x1800, 
    0x1880, 0x1900, 0x1980, 0x1a00, 0x1a80, 0x1b00, 0x1b80, 0x1c00, 0x1c80, 0x1d00, 0x1d80, 0x1e00, 0x1e80, 0x1f00, 0x1f80, 0x2000};
  bfloat16_t der_2_2[] = {  
    0x2080, 0x2100, 0x2180, 0x2200, 0x2280, 0x2300, 0x2380, 0x2400, 0x2480, 0x2500, 0x2580, 0x2600, 0x2680, 0x2700, 0x2780, 0x2800, 
    0x2880, 0x2900, 0x2980, 0x2a00, 0x2a80, 0x2b00, 0x2b80, 0x2c00, 0x2c80, 0x2d00, 0x2d80, 0x2e00, 0x2e80, 0x2f00, 0x2f80, 0x3000, 
    0x3080, 0x3100, 0x3180, 0x3200, 0x3280, 0x3300, 0x3380, 0x3400, 0x3480, 0x3500, 0x3580, 0x3600, 0x3680, 0x3700, 0x3780, 0x3800, 
    0x3880, 0x3900, 0x3980, 0x3a00, 0x3a80, 0x3b00, 0x3b80, 0x3c00, 0x3c80, 0x3d00, 0x3d80, 0x3e00, 0x3e80, 0x3f00, 0x3f80, 0x4000}; 
  bfloat16_t der_2_3[] = {  
    0x4080, 0x4100, 0x4180, 0x4200, 0x4280, 0x4300, 0x4380, 0x4400, 0x4480, 0x4500, 0x4580, 0x4600, 0x4680, 0x4700, 0x4780, 0x4800, 
    0x4880, 0x4900, 0x4980, 0x4a00, 0x4a80, 0x4b00, 0x4b80, 0x4c00, 0x4c80, 0x4d00, 0x4d80, 0x4e00, 0x4e80, 0x4f00, 0x4f80, 0x5000, 
    0x5080, 0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380, 0x5400, 0x5480, 0x5500, 0x5580, 0x5600, 0x5680, 0x5700, 0x5780, 0x5800, 
    0x5880, 0x5900, 0x5980, 0x5a00, 0x5a80, 0x5b00, 0x5b80, 0x5c00, 0x5c80, 0x5d00, 0x5d80, 0x5e00, 0x5e80, 0x5f00, 0x5f80, 0x6000};
  bfloat16_t der_2_4[] = {  
    0x6080, 0x6100, 0x6180, 0x6200, 0x6280, 0x6300, 0x6380, 0x6400, 0x6480, 0x6500, 0x6580, 0x6600, 0x6680, 0x6700, 0x6780, 0x6800, 
    0x6880, 0x6900, 0x6980, 0x6a00, 0x6a80, 0x6b00, 0x6b80, 0x6c00, 0x6c80, 0x6d00, 0x6d80, 0x6e00, 0x6e80, 0x6f00, 0x6f80, 0x7000, 
    0x7080, 0x7100, 0x7180, 0x7200, 0x7280, 0x7300, 0x7380, 0x7400, 0x7480, 0x7500, 0x7580, 0x7600, 0x7680, 0x7700, 0x7780, 0x7800, 
    0x7880, 0x7900, 0x7980, 0x7a00, 0x7a80, 0x7b00, 0x7b80, 0x7c00, 0x7c80, 0x7d00, 0x7d80, 0x7e00, 0x7e80 };
  bfloat16_t der_3_1[] = { 
    0x0000, 0x0000, 0x00ab, 0x012b, 0x01ab, 0x022b, 0x02ab, 0x032b, 0x03ab, 0x042b, 0x04ab, 0x052b, 0x05ab, 0x062b, 0x06ab, 0x072b, 
    0x07ab, 0x082b, 0x08ab, 0x092b, 0x09ab, 0x0a2b, 0x0aab, 0x0b2b, 0x0bab, 0x0c2b, 0x0cab, 0x0d2b, 0x0dab, 0x0e2b, 0x0eab, 0x0f2b, 
    0x0fab, 0x102b, 0x10ab, 0x112b, 0x11ab, 0x122b, 0x12ab, 0x132b, 0x13ab, 0x142b, 0x14ab, 0x152b, 0x15ab, 0x162b, 0x16ab, 0x172b, 
    0x17ab, 0x182b, 0x18ab, 0x192b, 0x19ab, 0x1a2b, 0x1aab, 0x1b2b, 0x1bab, 0x1c2b, 0x1cab, 0x1d2b, 0x1dab, 0x1e2b, 0x1eab, 0x1f2b}; 
  bfloat16_t der_3_2[] = { 
    0x1fab, 0x202b, 0x20ab, 0x212b, 0x21ab, 0x222b, 0x22ab, 0x232b, 0x23ab, 0x242b, 0x24ab, 0x252b, 0x25ab, 0x262b, 0x26ab, 0x272b, 
    0x27ab, 0x282b, 0x28ab, 0x292b, 0x29ab, 0x2a2b, 0x2aab, 0x2b2b, 0x2bab, 0x2c2b, 0x2cab, 0x2d2b, 0x2dab, 0x2e2b, 0x2eab, 0x2f2b, 
    0x2fab, 0x302b, 0x30ab, 0x312b, 0x31ab, 0x322b, 0x32ab, 0x332b, 0x33ab, 0x342b, 0x34ab, 0x352b, 0x35ab, 0x362b, 0x36ab, 0x372b, 
    0x37ab, 0x382b, 0x38ab, 0x392b, 0x39ab, 0x3a2b, 0x3aab, 0x3b2b, 0x3bab, 0x3c2b, 0x3cab, 0x3d2b, 0x3dab, 0x3e2b, 0x3eab, 0x3f2b}; 
  bfloat16_t der_3_3[] = { 
    0x3fab, 0x402b, 0x40ab, 0x412b, 0x41ab, 0x422b, 0x42ab, 0x432b, 0x43ab, 0x442b, 0x44ab, 0x452b, 0x45ab, 0x462b, 0x46ab, 0x472b, 
    0x47ab, 0x482b, 0x48ab, 0x492b, 0x49ab, 0x4a2b, 0x4aab, 0x4b2b, 0x4bab, 0x4c2b, 0x4cab, 0x4d2b, 0x4dab, 0x4e2b, 0x4eab, 0x4f2b, 
    0x4fab, 0x502b, 0x50ab, 0x512b, 0x51ab, 0x522b, 0x52ab, 0x532b, 0x53ab, 0x542b, 0x54ab, 0x552b, 0x55ab, 0x562b, 0x56ab, 0x572b, 
    0x57ab, 0x582b, 0x58ab, 0x592b, 0x59ab, 0x5a2b, 0x5aab, 0x5b2b, 0x5bab, 0x5c2b, 0x5cab, 0x5d2b, 0x5dab, 0x5e2b, 0x5eab, 0x5f2b}; 
  bfloat16_t der_3_4[] = {
    0x5fab, 0x602b, 0x60ab, 0x612b, 0x61ab, 0x622b, 0x62ab, 0x632b, 0x63ab, 0x642b, 0x64ab, 0x652b, 0x65ab, 0x662b, 0x66ab, 0x672b, 
    0x67ab, 0x682b, 0x68ab, 0x692b, 0x69ab, 0x6a2b, 0x6aab, 0x6b2b, 0x6bab, 0x6c2b, 0x6cab, 0x6d2b, 0x6dab, 0x6e2b, 0x6eab, 0x6f2b, 
    0x6fab, 0x702b, 0x70ab, 0x712b, 0x71ab, 0x722b, 0x72ab, 0x732b, 0x73ab, 0x742b, 0x74ab, 0x752b, 0x75ab, 0x762b, 0x76ab, 0x772b, 
    0x77ab, 0x782b, 0x78ab, 0x792b, 0x79ab, 0x7a2b, 0x7aab, 0x7b2b, 0x7bab, 0x7c2b, 0x7cab, 0x7d2b, 0x7dab };

  

  bfloat16_t val_n, der_1, der_2, der_3;
  if( n_int >= -125 && n_int <= -62 )
  {
    val_n = val_n_1[ n_int + 125 ];
    der_1 = der_1_1[ n_int + 125 ];
    der_2 = der_2_1[ n_int + 125 ];
    der_3 = der_3_1[ n_int + 125 ];
  }
  else if( n_int <= 2 )
  {
    val_n = val_n_2[ n_int + 61 ];
    der_1 = der_1_2[ n_int + 61 ];
    der_2 = der_2_2[ n_int + 61 ];
    der_3 = der_3_2[ n_int + 61 ];
  }
  else if ( n_int <= 66 )
  {
    val_n = val_n_3[ n_int - 3 ];
    der_1 = der_1_3[ n_int - 3 ];
    der_2 = der_2_3[ n_int - 3 ];
    der_3 = der_3_3[ n_int - 3 ];
  }
  else
  {
    val_n = val_n_4[ n_int - 67 ];
    der_1 = der_1_4[ n_int - 67 ];
    der_2 = der_2_4[ n_int - 67 ];
    der_3 = der_3_4[ n_int - 67 ];
  }

  uZ.f = bf16_add( val_n, bf16_mul( dx,
        bf16_add( der_1, bf16_mul( dx,
        bf16_add( der_2, bf16_mul( dx, 
        der_3 ) ) ) ) ) );
  
   
}