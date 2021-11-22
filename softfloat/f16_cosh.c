#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

float16_t f16_cosh( float16_t a )
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
    uiZ = 0x7c00;//+inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( a.v > 0xc9e4 )//a<-11.78125
  {
    uZ.ui = 0x7c00; //+inf
    return uZ.f;
  }
  if( ( a.v > 0x49e4 ) && ( a.v < 0x8000 ) ) //a>11.78125
  {
    uZ.ui = 0x7c00; //+inf
    return uZ.f;
  }
 //these numbers can't be computed precisely with the approximation method, so just give the values.
  if( ( ( a.v & ( 0x7FFF ) ) >= 0x49b8 ) && ( ( a.v & ( 0x7FFF ) ) <= 0x49e4 ) ) 
  {
    float16_t result[] = { 0x79a9, 0x79b4, 0x79c0, 0x79cb, 0x79d7, 0x79e3, 0x79ef, 0x79fa, 0x7a06, 0x7a13, 
                           0x7a1f, 0x7a2b, 0x7a37, 0x7a44, 0x7a50, 0x7a5d, 0x7a6a, 0x7a77, 0x7a84, 0x7a91, 
                           0x7a9e, 0x7aab, 0x7ab9, 0x7ac6, 0x7ad4, 0x7ae2, 0x7aef, 0x7afd, 0x7b0b, 0x7b19, 
                           0x7b28, 0x7b36, 0x7b45, 0x7b53, 0x7b62, 0x7b71, 0x7b80, 0x7b8f, 0x7b9e, 0x7bad, 
                           0x7bbd, 0x7bcc, 0x7bdc, 0x7beb, 0x7bfb }; 
    uZ.f = result[ ( a.v & 0x7FFF ) - 0x49b8 ];
    return uZ.f;
  }

  /** when x=nln2, the cosh(x) and all levels of deratives are easy to compute, 
   * so use the taylor series when x close to x0=nln2.
   */ 
  float16_t ln2_val, ln2_recip;
  ln2_val.v = 0x398c; // ln2=0.6931471805599453
  ln2_recip.v = 0x3dc5; // 1/ln2=1.4426950408889634

  float16_t n = f16_roundToInt( f16_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2)
  int16_t n_int = f16_to_i16( n, softfloat_round_near_maxMag, false );
  if(n_int < -16)
  {
      // frm = 2, a = 0xc9b7
      uZ.ui = 0x799e;
      return uZ.f;
  }
  float32_t x0 = f16_mul32( n, ln2_val ); // x0 = n * ln2
  float16_t dx = f32_to_f16( f32_sub( f16_to_f32( a ), x0 ) );

  //taylor series
  float16_t val_n[] = { 0x7800, 0x7400, 0x7000, 0x6c00, 0x6800, 0x6400, 0x6000, 0x5c00, 0x5800, 0x5400, 0x5000, 
                        0x4c01, 0x4804, 0x4410, 0x4040, 0x3d00, 0x3c00, 0x3d00, 0x4040, 0x4410, 0x4804, 0x4c01, 
                        0x5000, 0x5400, 0x5800, 0x5c00, 0x6000, 0x6400, 0x6800, 0x6c00, 0x7000, 0x7400, 0x7800 };
  float16_t der_1[] = { 0xf800, 0xf400, 0xf000, 0xec00, 0xe800, 0xe400, 0xe000, 0xdc00, 0xd800, 0xd400, 0xd000,
                        0xcbfe, 0xc7f8, 0xc3e0, 0xbf80, 0xba00, 0x0000, 0x3a00, 0x3f80, 0x43e0, 0x47f8, 0x4bfe,
                        0x5000, 0x5400, 0x5800, 0x5c00, 0x6000, 0x6400, 0x6800, 0x6c00, 0x7000, 0x7400, 0x7800 };
  float16_t der_2[] = { 0x7400, 0x7000, 0x6c00, 0x6800, 0x6400, 0x6000, 0x5c00, 0x5800, 0x5400, 0x5000, 0x4c00,
                        0x4801, 0x4404, 0x4010, 0x3c40, 0x3900, 0x3800, 0x3900, 0x3c40, 0x4010, 0x4404, 0x4801, 
                        0x4c00, 0x5000, 0x5400, 0x5800, 0x5c00, 0x6000, 0x6400, 0x6800, 0x6c00, 0x7000, 0x7400 };
  float16_t der_3[] = { 0xed55, 0xe955, 0xe555, 0xe155, 0xdd55, 0xd955, 0xd555, 0xd155, 0xcd55, 0xc955, 0xc555,
                        0xc154, 0xbd50, 0xb940, 0xb500, 0xb000, 0x0000, 0x3000, 0x3500, 0x3940, 0x3d50, 0x4154, 
                        0x4555, 0x4955, 0x4d55, 0x5155, 0x5555, 0x5955, 0x5d55, 0x6155, 0x6555, 0x6955, 0x6d55 };

  uZ.f = f16_add( val_n[ n_int + 16 ], f16_mul( dx,
         f16_add( der_1[ n_int + 16 ], f16_mul( dx,
         f16_add( der_2[ n_int + 16 ], f16_mul( dx, 
         der_3[ n_int + 16 ] ) ) ) ) ) );

  return uZ.f;
}