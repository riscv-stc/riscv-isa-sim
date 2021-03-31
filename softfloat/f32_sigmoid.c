#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"
#include <stdio.h>

float32_t f32_sigmoid( float32_t a )
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

  //if expA = 0xFF, handle the NaN and infinite number
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
    uiZ = signA ? 0x0 : 0x3f800000; // 0 or 1
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( a.v >= 0xc2d00000 ) //a<=-104
  {
    uZ.ui = 0;
    return uZ.f;
  }
  if( ( a.v >= 0x41900000 ) && ( a.v < 0x80000000 ) ) //a>=18
  {
    uZ.ui = 0x3f800000;
    return uZ.f;
  }

  /** when x=nln2, the sigmoid(x) and all levels of deratives are easy to compute, 
   * so use the taylor series when x close to x0=nln2.
   */ 
  float32_t ln2_val, ln2_recip;
  ln2_val.v = 0x3f317218; // ln2=0.6931471805599453
  ln2_recip.v = 0x3fb8aa3b; // 1/ln2=1.4426950408889634

  float32_t n = f32_roundToInt( f32_mul( a, ln2_recip ), softfloat_round_near_maxMag, false ); // n = round(x/ln2) -14~14
  int32_t n_int = f32_to_i32( n, softfloat_round_near_maxMag, false );
  float32_t x0 = f32_mul( n, ln2_val ); // x0 = n * ln2
  float32_t dx = f32_sub( a, x0 );

  float32_t val_n_1[] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 
    0x00000080, 0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 
    0x00008000, 0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 
    0x00800000, 0x01000000, 0x01800000, 0x02000000, 0x02800000, 0x03000000, 0x03800000, 0x04000000, 
    0x04800000, 0x05000000, 0x05800000, 0x06000000, 0x06800000, 0x07000000, 0x07800000, 0x08000000, 
    0x08800000, 0x09000000, 0x09800000, 0x0a000000, 0x0a800000, 0x0b000000, 0x0b800000, 0x0c000000, 
    0x0c800000, 0x0d000000, 0x0d800000, 0x0e000000, 0x0e800000, 0x0f000000, 0x0f800000, 0x10000000, 
    0x10800000, 0x11000000, 0x11800000, 0x12000000, 0x12800000, 0x13000000, 0x13800000, 0x14000000
  };
  float32_t val_n_2[] = {
    0x14800000, 0x15000000, 0x15800000, 0x16000000, 0x16800000, 0x17000000, 0x17800000, 0x18000000, 
    0x18800000, 0x19000000, 0x19800000, 0x1a000000, 0x1a800000, 0x1b000000, 0x1b800000, 0x1c000000, 
    0x1c800000, 0x1d000000, 0x1d800000, 0x1e000000, 0x1e800000, 0x1f000000, 0x1f800000, 0x20000000, 
    0x20800000, 0x21000000, 0x21800000, 0x22000000, 0x22800000, 0x23000000, 0x23800000, 0x24000000, 
    0x24800000, 0x25000000, 0x25800000, 0x26000000, 0x26800000, 0x27000000, 0x27800000, 0x28000000, 
    0x28800000, 0x29000000, 0x29800000, 0x2a000000, 0x2a800000, 0x2b000000, 0x2b800000, 0x2c000000, 
    0x2c800000, 0x2d000000, 0x2d800000, 0x2e000000, 0x2e800000, 0x2f000000, 0x2f800000, 0x30000000, 
    0x30800000, 0x31000000, 0x31800000, 0x32000000, 0x32800000, 0x33000000, 0x337fffff, 0x33fffffe
  };
  float32_t val_n_3[] = {
    0x347ffffc, 0x34fffff8, 0x357ffff0, 0x35ffffe0, 0x367fffc0, 0x36ffff80, 0x377fff00, 0x37fffe00, 
    0x387ffc00, 0x38fff800, 0x397ff001, 0x39ffe004, 0x3a7fc010, 0x3aff8040, 0x3b7f00ff, 0x3bfe03f8, 
    0x3c7c0fc1, 0x3cf83e10, 0x3d70f0f1, 0x3de38e39, 0x3e4ccccd, 0x3eaaaaab, 0x3f000000, 0x3f2aaaab, 
    0x3f4ccccd, 0x3f638e39, 0x3f70f0f1, 0x3f783e10, 0x3f7c0fc1, 0x3f7e03f8, 0x3f7f00ff, 0x3f7f8040, 
    0x3f7fc010, 0x3f7fe004, 0x3f7ff001, 0x3f7ff800, 0x3f7ffc00, 0x3f7ffe00, 0x3f7fff00, 0x3f7fff80, 
    0x3f7fffc0, 0x3f7fffe0, 0x3f7ffff0, 0x3f7ffff8, 0x3f7ffffc, 0x3f7ffffe, 0x3f7fffff, 0x3f800000, 
    0x3f800000
  };

  float32_t der_1_1[] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 
    0x00000080, 0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 
    0x00008000, 0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 
    0x00800000, 0x01000000, 0x01800000, 0x02000000, 0x02800000, 0x03000000, 0x03800000, 0x04000000, 
    0x04800000, 0x05000000, 0x05800000, 0x06000000, 0x06800000, 0x07000000, 0x07800000, 0x08000000, 
    0x08800000, 0x09000000, 0x09800000, 0x0a000000, 0x0a800000, 0x0b000000, 0x0b800000, 0x0c000000, 
    0x0c800000, 0x0d000000, 0x0d800000, 0x0e000000, 0x0e800000, 0x0f000000, 0x0f800000, 0x10000000, 
    0x10800000, 0x11000000, 0x11800000, 0x12000000, 0x12800000, 0x13000000, 0x13800000, 0x14000000
  };
  float32_t der_1_2[] = {
    0x14800000, 0x15000000, 0x15800000, 0x16000000, 0x16800000, 0x17000000, 0x17800000, 0x18000000, 
    0x18800000, 0x19000000, 0x19800000, 0x1a000000, 0x1a800000, 0x1b000000, 0x1b800000, 0x1c000000, 
    0x1c800000, 0x1d000000, 0x1d800000, 0x1e000000, 0x1e800000, 0x1f000000, 0x1f800000, 0x20000000, 
    0x20800000, 0x21000000, 0x21800000, 0x22000000, 0x22800000, 0x23000000, 0x23800000, 0x24000000, 
    0x24800000, 0x25000000, 0x25800000, 0x26000000, 0x26800000, 0x27000000, 0x27800000, 0x28000000, 
    0x28800000, 0x29000000, 0x29800000, 0x2a000000, 0x2a800000, 0x2b000000, 0x2b800000, 0x2c000000, 
    0x2c800000, 0x2d000000, 0x2d800000, 0x2e000000, 0x2e800000, 0x2f000000, 0x2f800000, 0x30000000, 
    0x30800000, 0x31000000, 0x31800000, 0x32000000, 0x32800000, 0x32ffffff, 0x337ffffe, 0x33fffffc
  };
  float32_t der_1_3[] = {
    0x347ffff8, 0x34fffff0, 0x357fffe0, 0x35ffffc0, 0x367fff80, 0x36ffff00, 0x377ffe00, 0x37fffc00, 
    0x387ff800, 0x38fff001, 0x397fe003, 0x39ffc00c, 0x3a7f8030, 0x3aff00c0, 0x3b7e02fc, 0x3bfc0be0, 
    0x3c782f05, 0x3cf0b84d, 0x3d62c4a7, 0x3dca4588, 0x3e23d70a, 0x3e638e39, 0x3e800000, 0x3e638e39, 
    0x3e23d70a, 0x3dca4588, 0x3d62c4a7, 0x3cf0b84d, 0x3c782f05, 0x3bfc0be0, 0x3b7e02fc, 0x3aff00c0, 
    0x3a7f8030, 0x39ffc00c, 0x397fe003, 0x38fff001, 0x387ff800, 0x37fffc00, 0x377ffe00, 0x36ffff00, 
    0x367fff80, 0x35ffffc0, 0x357fffe0, 0x34fffff0, 0x347ffff8, 0x33fffffc, 0x337ffffe, 0x32ffffff, 
    0x32800000
  };

  float32_t der_2_1[] = {
    0x00000000, 0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 
    0x00000040, 0x00000080, 0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 
    0x00004000, 0x00008000, 0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 
    0x00400000, 0x00800000, 0x01000000, 0x01800000, 0x02000000, 0x02800000, 0x03000000, 0x03800000, 
    0x04000000, 0x04800000, 0x05000000, 0x05800000, 0x06000000, 0x06800000, 0x07000000, 0x07800000, 
    0x08000000, 0x08800000, 0x09000000, 0x09800000, 0x0a000000, 0x0a800000, 0x0b000000, 0x0b800000, 
    0x0c000000, 0x0c800000, 0x0d000000, 0x0d800000, 0x0e000000, 0x0e800000, 0x0f000000, 0x0f800000, 
    0x10000000, 0x10800000, 0x11000000, 0x11800000, 0x12000000, 0x12800000, 0x13000000, 0x13800000
  };
  float32_t der_2_2[] = {
    0x14000000, 0x14800000, 0x15000000, 0x15800000, 0x16000000, 0x16800000, 0x17000000, 0x17800000, 
    0x18000000, 0x18800000, 0x19000000, 0x19800000, 0x1a000000, 0x1a800000, 0x1b000000, 0x1b800000, 
    0x1c000000, 0x1c800000, 0x1d000000, 0x1d800000, 0x1e000000, 0x1e800000, 0x1f000000, 0x1f800000, 
    0x20000000, 0x20800000, 0x21000000, 0x21800000, 0x22000000, 0x22800000, 0x23000000, 0x23800000, 
    0x24000000, 0x24800000, 0x25000000, 0x25800000, 0x26000000, 0x26800000, 0x27000000, 0x27800000, 
    0x28000000, 0x28800000, 0x29000000, 0x29800000, 0x2a000000, 0x2a800000, 0x2b000000, 0x2b800000, 
    0x2c000000, 0x2c800000, 0x2d000000, 0x2d800000, 0x2e000000, 0x2e800000, 0x2f000000, 0x2f800000, 
    0x30000000, 0x30800000, 0x31000000, 0x31800000, 0x31ffffff, 0x327ffffe, 0x32fffffc, 0x337ffff8
  };
  float32_t der_2_3[] = {
    0x33fffff0, 0x347fffe0, 0x34ffffc0, 0x357fff80, 0x35ffff00, 0x367ffe00, 0x36fffc00, 0x377ff800, 
    0x37fff001, 0x387fe002, 0x38ffc009, 0x397f8024, 0x39ff0090, 0x3a7e023e, 0x3afc08f0, 0x3b782382, 
    0x3bf08c18, 0x3c62217f, 0x3cc816ed, 0x3d1d5286, 0x3d449ba6, 0x3d17b426, 0x00000000, 0xbd17b426, 
    0xbd449ba6, 0xbd1d5286, 0xbcc816ed, 0xbc62217f, 0xbbf08c18, 0xbb782382, 0xbafc08f0, 0xba7e023e, 
    0xb9ff0090, 0xb97f8024, 0xb8ffc009, 0xb87fe002, 0xb7fff001, 0xb77ff800, 0xb6fffc00, 0xb67ffe00, 
    0xb5ffff00, 0xb57fff80, 0xb4ffffc0, 0xb47fffe0, 0xb3fffff0, 0xb37ffff8, 0xb2fffffc, 0xb27ffffe, 
    0xb1ffffff
  };

  float32_t der_3_1[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000003, 0x00000005, 0x0000000b, 
    0x00000015, 0x0000002b, 0x00000055, 0x000000ab, 0x00000155, 0x000002ab, 0x00000555, 0x00000aab, 
    0x00001555, 0x00002aab, 0x00005555, 0x0000aaab, 0x00015555, 0x0002aaab, 0x00055555, 0x000aaaab, 
    0x00155555, 0x002aaaab, 0x00555555, 0x00aaaaab, 0x012aaaab, 0x01aaaaab, 0x022aaaab, 0x02aaaaab, 
    0x032aaaab, 0x03aaaaab, 0x042aaaab, 0x04aaaaab, 0x052aaaab, 0x05aaaaab, 0x062aaaab, 0x06aaaaab, 
    0x072aaaab, 0x07aaaaab, 0x082aaaab, 0x08aaaaab, 0x092aaaab, 0x09aaaaab, 0x0a2aaaab, 0x0aaaaaab, 
    0x0b2aaaab, 0x0baaaaab, 0x0c2aaaab, 0x0caaaaab, 0x0d2aaaab, 0x0daaaaab, 0x0e2aaaab, 0x0eaaaaab, 
    0x0f2aaaab, 0x0faaaaab, 0x102aaaab, 0x10aaaaab, 0x112aaaab, 0x11aaaaab, 0x122aaaab, 0x12aaaaab
  };
  float32_t der_3_2[] = {
    0x132aaaab, 0x13aaaaab, 0x142aaaab, 0x14aaaaab, 0x152aaaab, 0x15aaaaab, 0x162aaaab, 0x16aaaaab, 
    0x172aaaab, 0x17aaaaab, 0x182aaaab, 0x18aaaaab, 0x192aaaab, 0x19aaaaab, 0x1a2aaaab, 0x1aaaaaab, 
    0x1b2aaaab, 0x1baaaaab, 0x1c2aaaab, 0x1caaaaab, 0x1d2aaaab, 0x1daaaaab, 0x1e2aaaab, 0x1eaaaaab, 
    0x1f2aaaab, 0x1faaaaab, 0x202aaaab, 0x20aaaaab, 0x212aaaab, 0x21aaaaab, 0x222aaaab, 0x22aaaaab, 
    0x232aaaab, 0x23aaaaab, 0x242aaaab, 0x24aaaaab, 0x252aaaab, 0x25aaaaab, 0x262aaaab, 0x26aaaaab, 
    0x272aaaab, 0x27aaaaab, 0x282aaaab, 0x28aaaaab, 0x292aaaab, 0x29aaaaab, 0x2a2aaaab, 0x2aaaaaab, 
    0x2b2aaaab, 0x2baaaaab, 0x2c2aaaab, 0x2caaaaab, 0x2d2aaaab, 0x2daaaaab, 0x2e2aaaab, 0x2eaaaaab, 
    0x2f2aaaab, 0x2faaaaaa, 0x302aaaaa, 0x30aaaaaa, 0x312aaaa9, 0x31aaaaa8, 0x322aaaa5, 0x32aaaaa0  
  };
  float32_t der_3_3[] = {
    0x332aaa95, 0x33aaaa80, 0x342aaa55, 0x34aaaa00, 0x352aa955, 0x35aaa800, 0x362aa555, 0x36aaa000, 
    0x372a9556, 0x37aa8004, 0x382a5567, 0x38aa0048, 0x39295675, 0x39a8047b, 0x3a25672b, 0x3aa046b0, 
    0x3b166afc, 0x3b842f9b, 0x3bc9eba2, 0x3bdbc07a, 0x3a8bcf65, 0xbc4a4588, 0xbcaaaaab, 0xbc4a4588, 
    0x3a8bcf65, 0x3bdbc07a, 0x3bc9eba2, 0x3b842f9b, 0x3b166afc, 0x3aa046b0, 0x3a25672b, 0x39a8047b, 
    0x39295675, 0x38aa0048, 0x382a5567, 0x37aa8004, 0x372a9556, 0x36aaa000, 0x362aa555, 0x35aaa800, 
    0x352aa955, 0x34aaaa00, 0x342aaa55, 0x33aaaa80, 0x332aaa95, 0x32aaaaa0, 0x322aaaa5, 0x31aaaaa8, 
    0x312aaaa9
  };

  float32_t valn, der1, der2, der3;
  if( n_int >= -150 && n_int <= -87 )
  {
    valn = val_n_1[n_int + 150];
    der1 = der_1_1[n_int + 150];
    der2 = der_2_1[n_int + 150];
    der3 = der_3_1[n_int + 150];
  }
  else if( n_int > -87 && n_int <= -23 )
  {
    valn = val_n_2[n_int + 86];
    der1 = der_1_2[n_int + 86];
    der2 = der_2_2[n_int + 86];
    der3 = der_3_2[n_int + 86];
  }
  else
  {
    valn = val_n_3[n_int + 22];
    der1 = der_1_3[n_int + 22];
    der2 = der_2_3[n_int + 22];
    der3 = der_3_3[n_int + 22];
  }

  uZ.f = f32_add( valn, f32_mul( dx,
         f32_add( der1, f32_mul( dx,
         f32_add( der2, f32_mul( dx, der3 ) ) ) ) ) );

  return uZ.f;
}


