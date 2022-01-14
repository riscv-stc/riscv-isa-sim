#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"


// float16_t f16_exp( float16_t a )
// {
  
//   union ui16_f16 uA;
//   uint_fast16_t uiA;
//   bool signA;
//   int_fast8_t expA;
//   uint_fast16_t sigA;

//   uint_fast16_t uiZ;
//   union ui16_f16 uZ;

//   //get the signal\exp\fraction bits from input float number a
//   uA.f = a;
//   uiA = uA.ui;
//   signA = signF16UI( uiA );
//   expA = expF16UI( uiA );
//   sigA = fracF16UI( uiA );

//   //if expA = 0x1f, handle the NaN and infinite number
//   if( 0x1F == expA )
//   {
//     //if NaN input
//     if( sigA )
//     {
//       uiZ = signA ? 0xfe00 : 0x7e00;//-NaN or NaN
//       uZ.ui = uiZ;
//       return uZ.f;
//     }
//     //infinite number input
//     uiZ = signA ? 0x0 : 0x7c00; // 0 or inf
//     uZ.ui = uiZ;
//     return uZ.f;
//   }

//   if( a.v > 0xcc55 )//a<-17.328125
//   {
//     uZ.ui = 0x0; //0
//     return uZ.f;
//   }
//   if( ( a.v > 0x498b ) && ( a.v < 0x8000 ) ) //a>11.0859375
//   {
//     uZ.ui = 0x7c00; //+inf
//     return uZ.f;
//   }
//   //these numbers can't be computed precisely with the approximation method, so just give the values.
//   if( ( a.v >= 0x4960 ) && ( a.v <= 0x498b ) )
//   {
//     float16_t result[] = { 0x79b1, 0x79bd, 0x79c8, 0x79d4, 0x79df, 0x79eb, 0x79f7, 0x7a03, 0x7a0f, 0x7a1b, 0x7a28, 0x7a34, 0x7a40, 0x7a4d, 0x7a5a, 0x7a66, 
//                            0x7a73, 0x7a80, 0x7a8d, 0x7a9a, 0x7aa8, 0x7ab5, 0x7ac2, 0x7ad0, 0x7ade, 0x7aeb, 0x7af9, 0x7b07, 0x7b16, 0x7b24, 0x7b32, 0x7b41, 
//                            0x7b4f, 0x7b5e, 0x7b6d, 0x7b7b, 0x7b8a, 0x7b9a, 0x7ba9, 0x7bb8, 0x7bc8, 0x7bd7, 0x7be7, 0x7bf7 };
//     uZ.f = result[ a.v - 0x4960 ];
//     return uZ.f;
//   }
  

  
//   /** when x=nln2, the exp(x) and all levels of deratives are easy to compute, 
//    * so use the taylor series when x close to x0=nln2.
//    */ 
//   float16_t ln2_val, ln2_recip;
//   ln2_val.v = 0x398c; // ln2=0.6931471805599453
//   ln2_recip.v = 0x3dc5; // 1/ln2=1.4426950408889634

//   float16_t n = f16_roundToInt( f16_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2)
//   int16_t n_int = f16_to_i16( n, softfloat_round_near_maxMag, false );
//   if( n_int > 15 )
//   {
//       // frm=3, a=0x495f
//       uZ.ui = 0x79a6;
//       return uZ.f;
//   }
//   float32_t x0 = f16_mul32( n, ln2_val ); // x0 = n * ln2
//   float16_t dx = f32_to_f16( f32_sub( f16_to_f32( a ), x0 ) );

//   //taylor series
//   float16_t val_n[] = { 0x0000, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 
//                         0x0400, 0x0800, 0x0c00, 0x1000, 0x1400, 0x1800, 0x1c00, 0x2000, 0x2400, 0x2800, 0x2c00, 
//                         0x3000, 0x3400, 0x3800, 0x3c00, 0x4000, 0x4400, 0x4800, 0x4c00, 0x5000, 0x5400, 0x5800, 
//                         0x5c00, 0x6000, 0x6400, 0x6800, 0x6c00, 0x7000, 0x7400, 0x7800 };
//   float16_t der_1[] = { 0x0000, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 
//                         0x0400, 0x0800, 0x0c00, 0x1000, 0x1400, 0x1800, 0x1c00, 0x2000, 0x2400, 0x2800, 0x2c00, 
//                         0x3000, 0x3400, 0x3800, 0x3c00, 0x4000, 0x4400, 0x4800, 0x4c00, 0x5000, 0x5400, 0x5800, 
//                         0x5c00, 0x6000, 0x6400, 0x6800, 0x6c00, 0x7000, 0x7400, 0x7800 };
//   float16_t der_2[] = { 0x0000, 0x0000, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 
//                         0x0200, 0x0400, 0x0800, 0x0c00, 0x1000, 0x1400, 0x1800, 0x1c00, 0x2000, 0x2400, 0x2800, 
//                         0x2c00, 0x3000, 0x3400, 0x3800, 0x3c00, 0x4000, 0x4400, 0x4800, 0x4c00, 0x5000, 0x5400, 
//                         0x5800, 0x5c00, 0x6000, 0x6400, 0x6800, 0x6c00, 0x7000, 0x7400 };
//   float16_t der_3[] = { 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0003, 0x0005, 0x000b, 0x0015, 0x002b, 0x0055, 
//                         0x00ab, 0x0155, 0x02ab, 0x0555, 0x0955, 0x0d55, 0x1155, 0x1555, 0x1955, 0x1d55, 0x2155, 
//                         0x2555, 0x2955, 0x2d55, 0x3155, 0x3555, 0x3955, 0x3d55, 0x4155, 0x4555, 0x4955, 0x4d55, 
//                         0x5155, 0x5555, 0x5955, 0x5d55, 0x6155, 0x6555, 0x6955, 0x6d55 };

//   uZ.f = f16_add( val_n[ n_int + 25 ], f16_mul( dx,
//          f16_add( der_1[ n_int + 25 ], f16_mul( dx,
//          f16_add( der_2[ n_int + 25 ], f16_mul( dx, 
//          der_3[ n_int + 25 ] ) ) ) ) ) );

//   return uZ.f;
// }

float16_t f16_exp( float16_t a )
{
  
  union ui16_f16 uA;
  uint_fast16_t uiA;
  bool signA;
  int_fast8_t expA;
  uint_fast16_t sigA;

  uint_fast16_t uiZ;
  union ui16_f16 uZ;

  //get the signal\exp\fraction bits from input float number a
  uA.f = a;
  uiA = uA.ui;
  signA = signF16UI( uiA );
  expA = expF16UI( uiA );
  sigA = fracF16UI( uiA );

  //if expA = 0x1f, handle the NaN and infinite number
  if( 0x1F == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xfe00 : 0x7e00;//-NaN or NaN
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0x0 : 0x7c00; // 0 or inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( a.v > 0xcc55 )//a<-17.328125
  {
    uZ.ui = 0x0; //0
    return uZ.f;
  }
  if( ( a.v > 0x498b ) && ( a.v < 0x8000 ) ) //a>11.0859375
  {
    uZ.ui = 0x7c00; //+inf
    return uZ.f;
  }

  
  /** when x=kln2+r, exp(kln2+r)=(2^k) * exp(r)
   * so use the taylor series to compute exp(r), then add k to the exp bits of exp(r)
   */ 
  float32_t ln2_val = { 0x3f317218 };
  float16_t ln2_recip = { 0x3dc5 };

  float16_t k = f16_roundToInt( f16_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // k = round(x/ln2)
  float32_t k_32 = f16_to_f32( k );
  float32_t a_32 = f16_to_f32( a );
  float32_t r_32 = f32_sub( a_32, f32_mul( k_32, ln2_val ) );
  float16_t r = f32_to_f16( r_32 );

  float16_t P1 = { 0x3c00 };  //1
  float16_t P2 = { 0x3800 };  //1/2
  float16_t P3 = { 0x3155 };  //1/6
  float16_t P4 = { 0x2955 };  //1/24

  r = f16_add( P1, f16_mul( r, 
  f16_add( P1, f16_mul( r,
  f16_add( P2, f16_mul( r, 
  f16_add( P3, f16_mul( r, P4 ) ) ) ) ) ) ) );

  int_fast16_t expz = f16_to_i16( k, softfloat_round_near_maxMag, false ) + ( ( ( int_fast16_t )( r.v & 0x7c00 ) ) >> 10 );
  if( expz < -10 )
  {
    uZ.ui = 0;
    return uZ.f;
  }
  else if( expz < 1 )
  { // subnormal number
    uZ.ui = ( ( r.v & 0x3FF ) + 0x400 ) >> ( -expz + 1 );

    // round the truncation bits
    if( ( ( ( r.v & 0x3FF ) + 0x400 ) & ( ( 1 << (-expz+1) ) - 1 ) )  >=  ( 1 << (-expz) )  )
    {
      uZ.ui += 1;
    }
    return uZ.f;
  }
  else if( expz > 30 )
  {//result is too big
    uZ.ui = 0x7c00;//+inf
    return uZ.f;
  }
  else
  { //compute 2^(z+f)
    uZ.ui = ( r.v & 0x83FF ) +  ( expz << 10 );
    return uZ.f;
  }   
}