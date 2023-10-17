#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"


float16_t f16_sigmoid( float16_t a )
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
    uiZ = signA ? 0x0 : 0x3c00; // 0 or 1
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( a.v >= 0xcc60 )//a<-17.5
  {
    uZ.ui = 0;
    return uZ.f;
  }
  if( ( a.v >= 0x4830 ) && ( a.v < 0x8000 ) ) //a>=8.375
  {
    uZ.ui = 0x3c00; //1
    return uZ.f;
  }

  /** when x=nln2, the sigmoid(x) and all levels of deratives are easy to compute, 
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
  float16_t val_n[] = { {0x0000}, {0x0001}, {0x0002}, {0x0004}, {0x0008}, {0x0010}, {0x0020}, {0x0040}, {0x0080}, {0x0100}, {0x0200}, 
                        {0x0400}, {0x0800}, {0x0c00}, {0x0fff}, {0x13fe}, {0x17fc}, {0x1bf8}, {0x1ff0}, {0x23e0}, {0x27c2}, {0x2b88}, 
                        {0x2f1c}, {0x3266}, {0x3555}, {0x3800}, {0x3955}, {0x3a66}, {0x3b1c}, {0x3b88}, {0x3bc2}, {0x3be0}, {0x3bf0}, 
                        {0x3bf8}, {0x3bfc}, {0x3bfe}, {0x3bff}, {0x3c00} };
  float16_t der_1[] = { {0x0000}, {0x0001}, {0x0002}, {0x0004}, {0x0008}, {0x0010}, {0x0020}, {0x0040}, {0x0080}, {0x0100}, {0x0200}, 
                        {0x0400}, {0x0800}, {0x0bff}, {0x0ffe}, {0x13fc}, {0x17f8}, {0x1bf0}, {0x1fe0}, {0x23c1}, {0x2786}, {0x2b16}, 
                        {0x2e52}, {0x311f}, {0x331c}, {0x3400}, {0x331c}, {0x311f}, {0x2e52}, {0x2b16}, {0x2786}, {0x23c1}, {0x1fe0}, 
                        {0x1bf0}, {0x17f8}, {0x13fc}, {0x0ffe}, {0x0bff} };
  float16_t der_2[] = { {0x0000}, {0x0000}, {0x0001}, {0x0002}, {0x0004}, {0x0008}, {0x0010}, {0x0020}, {0x0040}, {0x0080}, {0x0100}, 
                        {0x0200}, {0x0400}, {0x07fe}, {0x0bfc}, {0x0ff8}, {0x13f0}, {0x17e0}, {0x1bc1}, {0x1f84}, {0x2311}, {0x2641}, 
                        {0x28eb}, {0x2a25}, {0x28be}, {0x0000}, {0xa8be}, {0xaa25}, {0xa8eb}, {0xa641}, {0xa311}, {0x9f84}, {0x9bc1}, 
                        {0x97e0}, {0x93f0}, {0x8ff8}, {0x8bfc}, {0x87fe} };
  float16_t der_3[] = { {0x0000}, {0x0000}, {0x0000}, {0x0001}, {0x0001}, {0x0003}, {0x0005}, {0x000b}, {0x0015}, {0x002b}, {0x0055}, 
                        {0x00ab}, {0x0155}, {0x02a9}, {0x0550}, {0x094b}, {0x0d40}, {0x112b}, {0x1502}, {0x18b3}, {0x1c21}, {0x1e4f}, 
                        {0x1ede}, {0x145e}, {0xa252}, {0xa555}, {0xa252}, {0x145e}, {0x1ede}, {0x1e4f}, {0x1c21}, {0x18b3}, {0x1502}, 
                        {0x112b}, {0x0d40}, {0x094b}, {0x0550}, {0x02a9} };

  uZ.f = f16_add( val_n[ n_int + 25 ], f16_mul( dx,
         f16_add( der_1[ n_int + 25 ], f16_mul( dx,
         f16_add( der_2[ n_int + 25 ], f16_mul( dx, 
         der_3[ n_int + 25 ] ) ) ) ) ) );
  return uZ.f;
}