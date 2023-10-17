#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

float16_t f16_tanh( float16_t a )
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
    uiZ = signA ? 0xbc00 : 0x3c00;//-1 or 1
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( a.v > 0xc481 )//a<-4.5039
  {
    uZ.ui = 0xbc00; //-1
    return uZ.f;
  }
  if( ( a.v > 0x4481 ) && ( a.v < 0x8000 ) ) //a>4.5039
  {
    uZ.ui = 0x3c00; //1
    return uZ.f;
  }

  /** when x=nln2, the tanh(x) and all levels of deratives are easy to compute, 
   * so use the taylor series when x close to x0=nln2.
   */ 
  float16_t ln2_val, ln2_recip;
  ln2_val.v = 0x398c; // ln2=0.6931471805599453
  ln2_recip.v = 0x3dc5; // 1/ln2=1.4426950408889634

  float16_t n = f16_roundToInt( f16_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2)
  int16_t n_int = f16_to_i16( n, softfloat_round_near_maxMag, false );
  float32_t x0 = f16_mul32( n, ln2_val ); // x0 = n * ln2
  float16_t dx = f32_to_f16( f32_sub( f16_to_f32( a ), x0 ) );

  //taylor series
  float16_t val_n[] = { {0xbbff}, {0xbbfc}, {0xbbf0}, {0xbbc1}, {0xbb0f}, {0xb8cd}, {0x0000}, {0x38cd}, {0x3b0f}, {0x3bc1}, {0x3bf0}, {0x3bfc}, {0x3bff} };
  float16_t der_1[] = { {0x13ff}, {0x1bfc}, {0x23f0}, {0x2bc1}, {0x3316}, {0x391f}, {0x3c00}, {0x391f}, {0x3316}, {0x2bc1}, {0x23f0}, {0x1bfc}, {0x13ff} };
  float16_t der_2[] = { {0x13fe}, {0x1bf8}, {0x23e0}, {0x2b84}, {0x3241}, {0x3625}, {0x0000}, {0xb625}, {0xb241}, {0xab84}, {0xa3e0}, {0x9bf8}, {0x93fe} };
  float16_t der_3[] = { {0x1153}, {0x194b}, {0x212b}, {0x28b3}, {0x2e4f}, {0x245e}, {0xb555}, {0x245e}, {0x2e4f}, {0x28b3}, {0x212b}, {0x194b}, {0x1153} };


  uZ.f = f16_add( val_n[ n_int + 6 ], f16_mul( dx,
         f16_add( der_1[ n_int + 6 ], f16_mul( dx,
         f16_add( der_2[ n_int + 6 ], f16_mul( dx, 
         der_3[ n_int + 6 ] ) ) ) ) ) );

  if( ( ( a.v & 0x7FFF ) >= 0x358c ) && ( ( a.v & 0x7FFF ) <= 0x367c ))
  {
    uZ.ui += 5;
  }

  return uZ.f;
}