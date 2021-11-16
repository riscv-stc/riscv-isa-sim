#include "platform.h"
#include "softfloat.h"
#include "internals.h"
#include "specialize.h"

// float32_t f32_exp( float32_t a )
// {

//   float32_t ln2 = { 0x3f317218 } ; //0.69314718055994530941723212145818
//   float32_t one = { 0x3f800000 } ; //1
//   float32_t recip_ln2 = { 0x3fb8aa3b } ; //1.4426950408889634073599246810019
//   float32_t uP3 = { 0x3e75fdf0 }; //ln2^2/2
//   float32_t uP4 = { 0x3d635847 }; //ln2^3/6

//   float32_t x, z, f;

//   uint_fast32_t uiA;
//   bool signA;
//   int_fast16_t expA;
//   uint_fast32_t sigA;
//   union ui32_f32 uA;

//   uint_fast32_t uiZ;
//   union ui32_f32 uZ;

//   //get the signal\exp\fraction bits from input float number a
//   uA.f = a;
//   uiA = uA.ui;
//   signA = signF32UI( uiA );
//   expA = expF32UI( uiA );
//   sigA = fracF32UI( uiA );

//   //if expA = 0xFF, handle the NaN and infinite number
//   if( 0xFF == expA )
//   {
//     //if NaN input
//     if( sigA )
//     {
//       uiZ = signA ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
//       uZ.ui = uiZ;
//       return uZ.f;
//     }
//     //infinite number input
//     uiZ = signA ? 0x0 : uiA; // 0 or +inf
//     uZ.ui = uiZ;
//     return uZ.f;
//   }

//   if( a.v >= 0xc2d00000 ) //a<=-104
//   {
//     uZ.ui = 0;
//     return uZ.f;
//   }
//   //when a >= 89, the result will be +inf
//   if( ( a.v >= 0x42b20000 ) && ( a.v < 0x80000000 ) )
//   {
//     uZ.ui = 0x7f800000; //+inf
//     return uZ.f;
//   }


//   //compute e^a as 2^(a/ln(2))=2^(z+f) which |f|<1
//   x = f32_mul( a, recip_ln2 );
//   z = f32_roundToInt( x, softfloat_round_near_maxMag, false );
//   f = f32_sub( x, z );

//   //compute 2^f using taylor series
//   f = f32_add( one, f32_mul( f, 
//   f32_add( ln2, f32_mul( f, 
//   f32_add( uP3, f32_mul( f, uP4 ) ) ) ) ) );

//   //compute the exp bits of 2^(z+f)
//   int_fast32_t expz = f32_to_i32( z, softfloat_round_near_maxMag, false ) + ( ( ( int_fast32_t )( f.v & 0x7f800000 ) ) >> 23 );
//   if( expz < -23 )
//   {
//     uZ.ui = 0;
//     return uZ.f;
//   }
//   else if( expz < 1 )
//   {//result is too small
//     uZ.ui = ( ( f.v & 0x7FFFFF ) + 0x800000 ) >> ( -expz + 1 );
//     return uZ.f;
//   }
//   else if( expz > 254 )
//   {//result is too big
//     uZ.ui = 0x7f800000;//+inf
//     return uZ.f;
//   }
//   else
//   { //compute 2^(z+f)
//     uZ.ui = ( f.v & 0x807FFFFF ) +  ( expz << 23 );
//     return uZ.f;
//   } 
// }

float32_t f32_exp( float32_t a )
{

  uint_fast32_t uiA;
  bool signA;
  int_fast16_t expA;
  uint_fast32_t sigA;
  union ui32_f32 uA;

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
    uiZ = signA ? 0x0 : uiA; // 0 or +inf
    uZ.ui = uiZ;
    return uZ.f;
  }

  if( a.v >= 0xc2cff1b5 ) //a<=-104
  {
    uZ.ui = 0;
    return uZ.f;
  }
  else if ( a.v >= 0xc2cf0000 )
  {
    uZ.ui = 0x1;
    return uZ.f;    
  }
  
  //when a >= 89, the result will be +inf
  if( ( a.v >= 0x42b17218 ) && ( a.v < 0x80000000 ) )
  {
    uZ.ui = 0x7f800000; //+inf
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
  float32_t val_n_1[] = { 
  0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
  0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000, 
  0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000, 
  0x01000000, 0x01800000, 0x02000000, 0x02800000, 0x03000000, 0x03800000, 0x04000000, 0x04800000, 
  0x05000000, 0x05800000, 0x06000000, 0x06800000, 0x07000000, 0x07800000, 0x08000000, 0x08800000, 
  0x09000000, 0x09800000, 0x0a000000, 0x0a800000, 0x0b000000, 0x0b800000, 0x0c000000, 0x0c800000, 
  0x0d000000, 0x0d800000, 0x0e000000, 0x0e800000, 0x0f000000, 0x0f800000, 0x10000000, 0x10800000, 
  0x11000000, 0x11800000, 0x12000000, 0x12800000, 0x13000000, 0x13800000, 0x14000000, 0x14800000};
  float32_t val_n_2[] = {
  0x15000000, 0x15800000, 0x16000000, 0x16800000, 0x17000000, 0x17800000, 0x18000000, 0x18800000, 
  0x19000000, 0x19800000, 0x1a000000, 0x1a800000, 0x1b000000, 0x1b800000, 0x1c000000, 0x1c800000, 
  0x1d000000, 0x1d800000, 0x1e000000, 0x1e800000, 0x1f000000, 0x1f800000, 0x20000000, 0x20800000, 
  0x21000000, 0x21800000, 0x22000000, 0x22800000, 0x23000000, 0x23800000, 0x24000000, 0x24800000, 
  0x25000000, 0x25800000, 0x26000000, 0x26800000, 0x27000000, 0x27800000, 0x28000000, 0x28800000, 
  0x29000000, 0x29800000, 0x2a000000, 0x2a800000, 0x2b000000, 0x2b800000, 0x2c000000, 0x2c800000, 
  0x2d000000, 0x2d800000, 0x2e000000, 0x2e800000, 0x2f000000, 0x2f800000, 0x30000000, 0x30800000, 
  0x31000000, 0x31800000, 0x32000000, 0x32800000, 0x33000000, 0x33800000, 0x34000000, 0x34800000};
  float32_t val_n_3[] = {
  0x35000000, 0x35800000, 0x36000000, 0x36800000, 0x37000000, 0x37800000, 0x38000000, 0x38800000, 
  0x39000000, 0x39800000, 0x3a000000, 0x3a800000, 0x3b000000, 0x3b800000, 0x3c000000, 0x3c800000, 
  0x3d000000, 0x3d800000, 0x3e000000, 0x3e800000, 0x3f000000, 0x3f800000, 0x40000000, 0x40800000, 
  0x41000000, 0x41800000, 0x42000000, 0x42800000, 0x43000000, 0x43800000, 0x44000000, 0x44800000, 
  0x45000000, 0x45800000, 0x46000000, 0x46800000, 0x47000000, 0x47800000, 0x48000000, 0x48800000, 
  0x49000000, 0x49800000, 0x4a000000, 0x4a800000, 0x4b000000, 0x4b800000, 0x4c000000, 0x4c800000, 
  0x4d000000, 0x4d800000, 0x4e000000, 0x4e800000, 0x4f000000, 0x4f800000, 0x50000000, 0x50800000, 
  0x51000000, 0x51800000, 0x52000000, 0x52800000, 0x53000000, 0x53800000, 0x54000000, 0x54800000};
  float32_t val_n_4[] = {
  0x55000000, 0x55800000, 0x56000000, 0x56800000, 0x57000000, 0x57800000, 0x58000000, 0x58800000, 
  0x59000000, 0x59800000, 0x5a000000, 0x5a800000, 0x5b000000, 0x5b800000, 0x5c000000, 0x5c800000, 
  0x5d000000, 0x5d800000, 0x5e000000, 0x5e800000, 0x5f000000, 0x5f800000, 0x60000000, 0x60800000, 
  0x61000000, 0x61800000, 0x62000000, 0x62800000, 0x63000000, 0x63800000, 0x64000000, 0x64800000, 
  0x65000000, 0x65800000, 0x66000000, 0x66800000, 0x67000000, 0x67800000, 0x68000000, 0x68800000, 
  0x69000000, 0x69800000, 0x6a000000, 0x6a800000, 0x6b000000, 0x6b800000, 0x6c000000, 0x6c800000, 
  0x6d000000, 0x6d800000, 0x6e000000, 0x6e800000, 0x6f000000, 0x6f800000, 0x70000000, 0x70800000, 
  0x71000000, 0x71800000, 0x72000000, 0x72800000, 0x73000000, 0x73800000, 0x74000000, 0x74800000}; 
  float32_t val_n_5[] = {
  0x75000000, 0x75800000, 0x76000000, 0x76800000, 0x77000000, 0x77800000, 0x78000000, 0x78800000, 
  0x79000000, 0x79800000, 0x7a000000, 0x7a800000, 0x7b000000, 0x7b800000, 0x7c000000, 0x7c800000, 
  0x7d000000, 0x7d800000, 0x7e000000, 0x7e800000, 0x7f000000 };

  float32_t der_1_1[] = { 
  0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080, 
  0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000, 
  0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000, 
  0x01000000, 0x01800000, 0x02000000, 0x02800000, 0x03000000, 0x03800000, 0x04000000, 0x04800000, 
  0x05000000, 0x05800000, 0x06000000, 0x06800000, 0x07000000, 0x07800000, 0x08000000, 0x08800000, 
  0x09000000, 0x09800000, 0x0a000000, 0x0a800000, 0x0b000000, 0x0b800000, 0x0c000000, 0x0c800000, 
  0x0d000000, 0x0d800000, 0x0e000000, 0x0e800000, 0x0f000000, 0x0f800000, 0x10000000, 0x10800000, 
  0x11000000, 0x11800000, 0x12000000, 0x12800000, 0x13000000, 0x13800000, 0x14000000, 0x14800000};
  float32_t der_1_2[] = {
  0x15000000, 0x15800000, 0x16000000, 0x16800000, 0x17000000, 0x17800000, 0x18000000, 0x18800000, 
  0x19000000, 0x19800000, 0x1a000000, 0x1a800000, 0x1b000000, 0x1b800000, 0x1c000000, 0x1c800000, 
  0x1d000000, 0x1d800000, 0x1e000000, 0x1e800000, 0x1f000000, 0x1f800000, 0x20000000, 0x20800000, 
  0x21000000, 0x21800000, 0x22000000, 0x22800000, 0x23000000, 0x23800000, 0x24000000, 0x24800000, 
  0x25000000, 0x25800000, 0x26000000, 0x26800000, 0x27000000, 0x27800000, 0x28000000, 0x28800000, 
  0x29000000, 0x29800000, 0x2a000000, 0x2a800000, 0x2b000000, 0x2b800000, 0x2c000000, 0x2c800000, 
  0x2d000000, 0x2d800000, 0x2e000000, 0x2e800000, 0x2f000000, 0x2f800000, 0x30000000, 0x30800000, 
  0x31000000, 0x31800000, 0x32000000, 0x32800000, 0x33000000, 0x33800000, 0x34000000, 0x34800000}; 
  float32_t der_1_3[] = {
  0x35000000, 0x35800000, 0x36000000, 0x36800000, 0x37000000, 0x37800000, 0x38000000, 0x38800000, 
  0x39000000, 0x39800000, 0x3a000000, 0x3a800000, 0x3b000000, 0x3b800000, 0x3c000000, 0x3c800000, 
  0x3d000000, 0x3d800000, 0x3e000000, 0x3e800000, 0x3f000000, 0x3f800000, 0x40000000, 0x40800000, 
  0x41000000, 0x41800000, 0x42000000, 0x42800000, 0x43000000, 0x43800000, 0x44000000, 0x44800000, 
  0x45000000, 0x45800000, 0x46000000, 0x46800000, 0x47000000, 0x47800000, 0x48000000, 0x48800000, 
  0x49000000, 0x49800000, 0x4a000000, 0x4a800000, 0x4b000000, 0x4b800000, 0x4c000000, 0x4c800000, 
  0x4d000000, 0x4d800000, 0x4e000000, 0x4e800000, 0x4f000000, 0x4f800000, 0x50000000, 0x50800000, 
  0x51000000, 0x51800000, 0x52000000, 0x52800000, 0x53000000, 0x53800000, 0x54000000, 0x54800000}; 
  float32_t der_1_4[] = {
  0x55000000, 0x55800000, 0x56000000, 0x56800000, 0x57000000, 0x57800000, 0x58000000, 0x58800000, 
  0x59000000, 0x59800000, 0x5a000000, 0x5a800000, 0x5b000000, 0x5b800000, 0x5c000000, 0x5c800000, 
  0x5d000000, 0x5d800000, 0x5e000000, 0x5e800000, 0x5f000000, 0x5f800000, 0x60000000, 0x60800000, 
  0x61000000, 0x61800000, 0x62000000, 0x62800000, 0x63000000, 0x63800000, 0x64000000, 0x64800000, 
  0x65000000, 0x65800000, 0x66000000, 0x66800000, 0x67000000, 0x67800000, 0x68000000, 0x68800000, 
  0x69000000, 0x69800000, 0x6a000000, 0x6a800000, 0x6b000000, 0x6b800000, 0x6c000000, 0x6c800000, 
  0x6d000000, 0x6d800000, 0x6e000000, 0x6e800000, 0x6f000000, 0x6f800000, 0x70000000, 0x70800000, 
  0x71000000, 0x71800000, 0x72000000, 0x72800000, 0x73000000, 0x73800000, 0x74000000, 0x74800000}; 
  float32_t der_1_5[] = {
  0x75000000, 0x75800000, 0x76000000, 0x76800000, 0x77000000, 0x77800000, 0x78000000, 0x78800000, 
  0x79000000, 0x79800000, 0x7a000000, 0x7a800000, 0x7b000000, 0x7b800000, 0x7c000000, 0x7c800000, 
  0x7d000000, 0x7d800000, 0x7e000000, 0x7e800000, 0x7f000000 };

  float32_t der_2_1[] = { 
  0x00000000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 
  0x00000080, 0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 
  0x00008000, 0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 
  0x00800000, 0x01000000, 0x01800000, 0x02000000, 0x02800000, 0x03000000, 0x03800000, 0x04000000, 
  0x04800000, 0x05000000, 0x05800000, 0x06000000, 0x06800000, 0x07000000, 0x07800000, 0x08000000, 
  0x08800000, 0x09000000, 0x09800000, 0x0a000000, 0x0a800000, 0x0b000000, 0x0b800000, 0x0c000000, 
  0x0c800000, 0x0d000000, 0x0d800000, 0x0e000000, 0x0e800000, 0x0f000000, 0x0f800000, 0x10000000, 
  0x10800000, 0x11000000, 0x11800000, 0x12000000, 0x12800000, 0x13000000, 0x13800000, 0x14000000}; 
  float32_t der_2_2[] = { 
  0x14800000, 0x15000000, 0x15800000, 0x16000000, 0x16800000, 0x17000000, 0x17800000, 0x18000000, 
  0x18800000, 0x19000000, 0x19800000, 0x1a000000, 0x1a800000, 0x1b000000, 0x1b800000, 0x1c000000, 
  0x1c800000, 0x1d000000, 0x1d800000, 0x1e000000, 0x1e800000, 0x1f000000, 0x1f800000, 0x20000000, 
  0x20800000, 0x21000000, 0x21800000, 0x22000000, 0x22800000, 0x23000000, 0x23800000, 0x24000000, 
  0x24800000, 0x25000000, 0x25800000, 0x26000000, 0x26800000, 0x27000000, 0x27800000, 0x28000000, 
  0x28800000, 0x29000000, 0x29800000, 0x2a000000, 0x2a800000, 0x2b000000, 0x2b800000, 0x2c000000, 
  0x2c800000, 0x2d000000, 0x2d800000, 0x2e000000, 0x2e800000, 0x2f000000, 0x2f800000, 0x30000000, 
  0x30800000, 0x31000000, 0x31800000, 0x32000000, 0x32800000, 0x33000000, 0x33800000, 0x34000000}; 
  float32_t der_2_3[] = { 
  0x34800000, 0x35000000, 0x35800000, 0x36000000, 0x36800000, 0x37000000, 0x37800000, 0x38000000, 
  0x38800000, 0x39000000, 0x39800000, 0x3a000000, 0x3a800000, 0x3b000000, 0x3b800000, 0x3c000000, 
  0x3c800000, 0x3d000000, 0x3d800000, 0x3e000000, 0x3e800000, 0x3f000000, 0x3f800000, 0x40000000, 
  0x40800000, 0x41000000, 0x41800000, 0x42000000, 0x42800000, 0x43000000, 0x43800000, 0x44000000, 
  0x44800000, 0x45000000, 0x45800000, 0x46000000, 0x46800000, 0x47000000, 0x47800000, 0x48000000, 
  0x48800000, 0x49000000, 0x49800000, 0x4a000000, 0x4a800000, 0x4b000000, 0x4b800000, 0x4c000000, 
  0x4c800000, 0x4d000000, 0x4d800000, 0x4e000000, 0x4e800000, 0x4f000000, 0x4f800000, 0x50000000, 
  0x50800000, 0x51000000, 0x51800000, 0x52000000, 0x52800000, 0x53000000, 0x53800000, 0x54000000}; 
  float32_t der_2_4[] = { 
  0x54800000, 0x55000000, 0x55800000, 0x56000000, 0x56800000, 0x57000000, 0x57800000, 0x58000000, 
  0x58800000, 0x59000000, 0x59800000, 0x5a000000, 0x5a800000, 0x5b000000, 0x5b800000, 0x5c000000, 
  0x5c800000, 0x5d000000, 0x5d800000, 0x5e000000, 0x5e800000, 0x5f000000, 0x5f800000, 0x60000000, 
  0x60800000, 0x61000000, 0x61800000, 0x62000000, 0x62800000, 0x63000000, 0x63800000, 0x64000000, 
  0x64800000, 0x65000000, 0x65800000, 0x66000000, 0x66800000, 0x67000000, 0x67800000, 0x68000000, 
  0x68800000, 0x69000000, 0x69800000, 0x6a000000, 0x6a800000, 0x6b000000, 0x6b800000, 0x6c000000, 
  0x6c800000, 0x6d000000, 0x6d800000, 0x6e000000, 0x6e800000, 0x6f000000, 0x6f800000, 0x70000000, 
  0x70800000, 0x71000000, 0x71800000, 0x72000000, 0x72800000, 0x73000000, 0x73800000, 0x74000000};
  float32_t der_2_5[] = { 
  0x74800000, 0x75000000, 0x75800000, 0x76000000, 0x76800000, 0x77000000, 0x77800000, 0x78000000, 
  0x78800000, 0x79000000, 0x79800000, 0x7a000000, 0x7a800000, 0x7b000000, 0x7b800000, 0x7c000000, 
  0x7c800000, 0x7d000000, 0x7d800000, 0x7e000000, 0x7e800000 };

  float32_t der_3_1[] = { 
  0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000003, 0x00000005, 0x0000000b, 0x00000015, 
  0x0000002b, 0x00000055, 0x000000ab, 0x00000155, 0x000002ab, 0x00000555, 0x00000aab, 0x00001555, 
  0x00002aab, 0x00005555, 0x0000aaab, 0x00015555, 0x0002aaab, 0x00055555, 0x000aaaab, 0x00155555, 
  0x002aaaab, 0x00555555, 0x00aaaaab, 0x012aaaab, 0x01aaaaab, 0x022aaaab, 0x02aaaaab, 0x032aaaab, 
  0x03aaaaab, 0x042aaaab, 0x04aaaaab, 0x052aaaab, 0x05aaaaab, 0x062aaaab, 0x06aaaaab, 0x072aaaab, 
  0x07aaaaab, 0x082aaaab, 0x08aaaaab, 0x092aaaab, 0x09aaaaab, 0x0a2aaaab, 0x0aaaaaab, 0x0b2aaaab, 
  0x0baaaaab, 0x0c2aaaab, 0x0caaaaab, 0x0d2aaaab, 0x0daaaaab, 0x0e2aaaab, 0x0eaaaaab, 0x0f2aaaab, 
  0x0faaaaab, 0x102aaaab, 0x10aaaaab, 0x112aaaab, 0x11aaaaab, 0x122aaaab, 0x12aaaaab, 0x132aaaab}; 
  float32_t der_3_2[] = { 
  0x13aaaaab, 0x142aaaab, 0x14aaaaab, 0x152aaaab, 0x15aaaaab, 0x162aaaab, 0x16aaaaab, 0x172aaaab, 
  0x17aaaaab, 0x182aaaab, 0x18aaaaab, 0x192aaaab, 0x19aaaaab, 0x1a2aaaab, 0x1aaaaaab, 0x1b2aaaab, 
  0x1baaaaab, 0x1c2aaaab, 0x1caaaaab, 0x1d2aaaab, 0x1daaaaab, 0x1e2aaaab, 0x1eaaaaab, 0x1f2aaaab, 
  0x1faaaaab, 0x202aaaab, 0x20aaaaab, 0x212aaaab, 0x21aaaaab, 0x222aaaab, 0x22aaaaab, 0x232aaaab, 
  0x23aaaaab, 0x242aaaab, 0x24aaaaab, 0x252aaaab, 0x25aaaaab, 0x262aaaab, 0x26aaaaab, 0x272aaaab, 
  0x27aaaaab, 0x282aaaab, 0x28aaaaab, 0x292aaaab, 0x29aaaaab, 0x2a2aaaab, 0x2aaaaaab, 0x2b2aaaab, 
  0x2baaaaab, 0x2c2aaaab, 0x2caaaaab, 0x2d2aaaab, 0x2daaaaab, 0x2e2aaaab, 0x2eaaaaab, 0x2f2aaaab, 
  0x2faaaaab, 0x302aaaab, 0x30aaaaab, 0x312aaaab, 0x31aaaaab, 0x322aaaab, 0x32aaaaab, 0x332aaaab}; 
  float32_t der_3_3[] = { 
  0x33aaaaab, 0x342aaaab, 0x34aaaaab, 0x352aaaab, 0x35aaaaab, 0x362aaaab, 0x36aaaaab, 0x372aaaab, 
  0x37aaaaab, 0x382aaaab, 0x38aaaaab, 0x392aaaab, 0x39aaaaab, 0x3a2aaaab, 0x3aaaaaab, 0x3b2aaaab, 
  0x3baaaaab, 0x3c2aaaab, 0x3caaaaab, 0x3d2aaaab, 0x3daaaaab, 0x3e2aaaab, 0x3eaaaaab, 0x3f2aaaab, 
  0x3faaaaab, 0x402aaaab, 0x40aaaaab, 0x412aaaab, 0x41aaaaab, 0x422aaaab, 0x42aaaaab, 0x432aaaab, 
  0x43aaaaab, 0x442aaaab, 0x44aaaaab, 0x452aaaab, 0x45aaaaab, 0x462aaaab, 0x46aaaaab, 0x472aaaab, 
  0x47aaaaab, 0x482aaaab, 0x48aaaaab, 0x492aaaab, 0x49aaaaab, 0x4a2aaaab, 0x4aaaaaab, 0x4b2aaaab, 
  0x4baaaaab, 0x4c2aaaab, 0x4caaaaab, 0x4d2aaaab, 0x4daaaaab, 0x4e2aaaab, 0x4eaaaaab, 0x4f2aaaab, 
  0x4faaaaab, 0x502aaaab, 0x50aaaaab, 0x512aaaab, 0x51aaaaab, 0x522aaaab, 0x52aaaaab, 0x532aaaab}; 
  float32_t der_3_4[] = { 
  0x53aaaaab, 0x542aaaab, 0x54aaaaab, 0x552aaaab, 0x55aaaaab, 0x562aaaab, 0x56aaaaab, 0x572aaaab, 
  0x57aaaaab, 0x582aaaab, 0x58aaaaab, 0x592aaaab, 0x59aaaaab, 0x5a2aaaab, 0x5aaaaaab, 0x5b2aaaab, 
  0x5baaaaab, 0x5c2aaaab, 0x5caaaaab, 0x5d2aaaab, 0x5daaaaab, 0x5e2aaaab, 0x5eaaaaab, 0x5f2aaaab, 
  0x5faaaaab, 0x602aaaab, 0x60aaaaab, 0x612aaaab, 0x61aaaaab, 0x622aaaab, 0x62aaaaab, 0x632aaaab, 
  0x63aaaaab, 0x642aaaab, 0x64aaaaab, 0x652aaaab, 0x65aaaaab, 0x662aaaab, 0x66aaaaab, 0x672aaaab, 
  0x67aaaaab, 0x682aaaab, 0x68aaaaab, 0x692aaaab, 0x69aaaaab, 0x6a2aaaab, 0x6aaaaaab, 0x6b2aaaab, 
  0x6baaaaab, 0x6c2aaaab, 0x6caaaaab, 0x6d2aaaab, 0x6daaaaab, 0x6e2aaaab, 0x6eaaaaab, 0x6f2aaaab, 
  0x6faaaaab, 0x702aaaab, 0x70aaaaab, 0x712aaaab, 0x71aaaaab, 0x722aaaab, 0x72aaaaab, 0x732aaaab}; 
  float32_t der_3_5[] = { 
  0x73aaaaab, 0x742aaaab, 0x74aaaaab, 0x752aaaab, 0x75aaaaab, 0x762aaaab, 0x76aaaaab, 0x772aaaab, 
  0x77aaaaab, 0x782aaaab, 0x78aaaaab, 0x792aaaab, 0x79aaaaab, 0x7a2aaaab, 0x7aaaaaab, 0x7b2aaaab, 
  0x7baaaaab, 0x7c2aaaab, 0x7caaaaab, 0x7d2aaaab, 0x7daaaaab };

  float32_t val_n, der_1, der_2, der_3;
  if ( 128 == n_int )
  {//taylor coefficients can't expressed in f32, 
   //so when n_int= 128, use exp(128ln2+dx) to compute the result
    val_n.v = 0x3f800000 ;
    der_1.v = 0x3f800000 ;
    der_2.v = 0x3f000000 ;
    der_3.v = 0x3e2aaaab ;
  }
  else
  {  
    if( n_int >= -149 && n_int <= -86 )
    {
      val_n = val_n_1[ n_int + 149 ];
      der_1 = der_1_1[ n_int + 149 ];
      der_2 = der_2_1[ n_int + 149 ];
      der_3 = der_3_1[ n_int + 149 ];
    }
    else if( n_int < -21 )
    {
      val_n = val_n_2[ n_int + 85 ];
      der_1 = der_1_2[ n_int + 85 ];
      der_2 = der_2_2[ n_int + 85 ];
      der_3 = der_3_2[ n_int + 85 ];
    }
    else if ( n_int < 43 )
    {
      val_n = val_n_3[ n_int + 21 ];
      der_1 = der_1_3[ n_int + 21 ];
      der_2 = der_2_3[ n_int + 21 ];
      der_3 = der_3_3[ n_int + 21 ];
    }
    else if( n_int < 107 )
    {
      val_n = val_n_4[ n_int - 43 ];
      der_1 = der_1_4[ n_int - 43 ];
      der_2 = der_2_4[ n_int - 43 ];
      der_3 = der_3_4[ n_int - 43 ];
    }
    else
    {
      val_n = val_n_5[ n_int - 107 ];
      der_1 = der_1_5[ n_int - 107 ];
      der_2 = der_2_5[ n_int - 107 ];
      der_3 = der_3_5[ n_int - 107 ];    
    }
  }

  uZ.f = f32_add( val_n, f32_mul( dx,
        f32_add( der_1, f32_mul( dx,
        f32_add( der_2, f32_mul( dx, 
        der_3 ) ) ) ) ) );
  
  if ( 128 == n_int )
  {
    int16_t expZ = uZ.ui >> 23;
    expZ += 128;
    if( expZ > 254 )
    {
      uZ.ui = 0x7f800000;
    }
    else
    {
      uZ.ui = ((uint32_t) (128)<<23) + uZ.ui; 
    }  
  }

  return uZ.f;

}