#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

float32_t f32_tanh( float32_t a )
{

  union ui32_f32 uP1;
  union ui32_f32 uP2;
  union ui32_f32 uP3;
  union ui32_f32 uP4;
  union ui32_f32 uP5;
  union ui32_f32 uP6;

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

  //if expA = 0xff, handle the NaN and infinite number
  if( 0xFF == expA )
  {
    //if NaN input
    if( sigA )
    {
      uiZ = signA ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
      uZ.ui = uiZ;
      return uZ.f;
    }
    //infinite number input
    uiZ = signA ? 0xbf800000 : 0x3f800000;//-1 or 1
    uZ.ui = uiZ;
    return uZ.f;
  }

  if ( ( 0xc1200000 < a.v ) )// a < -10
  {
    uZ.ui = 0xbf800000;//-1
    return uZ.f;
  }
  if ( ( 0x41200000 <= a.v ) && ( 0x80000000 > a.v ))//a>=10
  {
    uZ.ui = 0x3f800000;//1
    return uZ.f;
  }

  /** when x=nln2, the tanh(x) and all levels of deratives are easy to compute, 
   * so use the taylor series when x close to x0=nln2.
   */ 
  float32_t ln2_val, ln2_recip;
  ln2_val.v = 0x3f317218; // ln2=0.6931471805599453
  ln2_recip.v = 0x3fb8aa3b; // 1/ln2=1.4426950408889634


  float32_t n = f32_roundToInt( f32_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2) -14~14
  int32_t n_int = f32_to_i32( n, softfloat_round_near_maxMag, false );
  float32_t x0 = f32_mul( n, ln2_val ); // x0 = n * ln2
  float32_t dx = f32_sub( a, x0 );


  //taylor series
  float32_t val_n[] = { 0xbf800000, 0xbf800000, 0xbf7ffffe, 0xbf7ffff8, 0xbf7fffe0, 0xbf7fff80, 0xbf7ffe00, 0xbf7ff800, 
                        0xbf7fe002, 0xbf7f8020, 0xbf7e01fe, 0xbf781f82, 0xbf61e1e2, 0xbf19999a, 0x00000000, 0x3f19999a, 
                        0x3f61e1e2, 0x3f781f82, 0x3f7e01fe, 0x3f7f8020, 0x3f7fe002, 0x3f7ff800, 0x3f7ffe00, 0x3f7fff80, 
                        0x3f7fffe0, 0x3f7ffff8, 0x3f7ffffe, 0x3f800000, 0x3f800000 };
  float32_t der_1[] = { 0x32800000, 0x33800000, 0x347ffffe, 0x357ffff8, 0x367fffe0, 0x377fff80, 0x387ffe00, 0x397ff800, 
                        0x3a7fe003, 0x3b7f8030, 0x3c7e02fc, 0x3d782f05, 0x3e62c4a7, 0x3f23d70a, 0x3f800000, 0x3f23d70a, 
                        0x3e62c4a7, 0x3d782f05, 0x3c7e02fc, 0x3b7f8030, 0x3a7fe003, 0x397ff800, 0x387ffe00, 0x377fff80, 
                        0x367fffe0, 0x357ffff8, 0x347ffffe, 0x33800000, 0x32800000 };
  float32_t der_2[] = { 0x32800000, 0x337fffff, 0x347ffffc, 0x357ffff0, 0x367fffc0, 0x377fff00, 0x387ffc00, 0x397ff001, 
                        0x3a7fc009, 0x3b7f0090, 0x3c7c08f0, 0x3d708c18, 0x3e4816ed, 0x3ec49ba6, 0x00000000, 0xbec49ba6, 
                        0xbe4816ed, 0xbd708c18, 0xbc7c08f0, 0xbb7f0090, 0xba7fc009, 0xb97ff001, 0xb87ffc00, 0xb77fff00, 
                        0xb67fffc0, 0xb57ffff0, 0xb47ffffc, 0xb37fffff, 0xb2800000 };
  float32_t der_3[] = { 0x322aaaaa, 0x332aaaa9, 0x342aaaa5, 0x352aaa95, 0x362aaa55, 0x372aa955, 0x382aa555, 0x392a9556, 
                        0x3a2a5567, 0x3b295675, 0x3c25672b, 0x3d166afc, 0x3dc9eba2, 0x3c8bcf65, 0xbeaaaaab, 0x3c8bcf65, 
                        0x3dc9eba2, 0x3d166afc, 0x3c25672b, 0x3b295675, 0x3a2a5567, 0x392a9556, 0x382aa555, 0x372aa955, 
                        0x362aaa55, 0x352aaa95, 0x342aaaa5, 0x332aaaa9, 0x322aaaaa };
  float32_t der_4[] = { 0x31aaaaaa, 0x32aaaaa8, 0x33aaaaa0, 0x34aaaa80, 0x35aaaa00, 0x36aaa800, 0x37aaa000, 0x38aa8003, 
                        0x39aa0036, 0x3aa8035d, 0x3ba03557, 0x3c8336e9, 0x3cb31696, 0xbdf12c28, 0x00000000, 0x3df12c28, 
                        0xbcb31696, 0xbc8336e9, 0xbba03557, 0xbaa8035d, 0xb9aa0036, 0xb8aa8003, 0xb7aaa000, 0xb6aaa800, 
                        0xb5aaaa00, 0xb4aaaa80, 0xb3aaaaa0, 0xb2aaaaa8, 0xb1aaaaaa };                        

  uZ.f = f32_add( val_n[ n_int + 14 ], f32_mul( dx,
        f32_add( der_1[ n_int + 14 ], f32_mul( dx,
        f32_add( der_2[ n_int + 14 ], f32_mul( dx, 
        f32_add( der_3[ n_int + 14 ], f32_mul( dx,
        der_4[ n_int + 14 ] ) ) ) ) ) ) ) );

  // make up to make results more accurate   
  if( ( ( a.v & 0x7FFFFFFF ) >= 0x3e8f5c29 ) && ( ( a.v & 0x7FFFFFFF ) <= 0x3eb33333 ))
  {
    uZ.ui += 15000;
  }
  return uZ.f;
  
}
