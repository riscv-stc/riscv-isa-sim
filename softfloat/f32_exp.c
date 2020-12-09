
#include "softfloat.h"
#include "internals.h"

float32_t f32_exp( float32_t a )
{
  union ui32_f32 uA;
  union ui32_f32 uC;
  union ui32_f32 uD;
  uint_fast8_t roundingMode;
  bool exact;

  /**
   * 
   * float number can be regarded as float number or integer
   * | S  |  E  |   M  |
   * [1bit][5bit][10bit]
   * when we regard it as a float number, its value:
   * (-1)^S*(1+M)*2^(E-bias)
   * when we reagrd it as a integer, its value:
   * S * 2^offset_s + M * 2^offset_M + E * 2^offset_E
   * 
   * So when we assign S = 0， M =0
   * float number <-> integer
   *  2^(E-bias) <-> E * 2^offset
   * So,
   * float number <-> integer
   * 2^X          <-> 2^offset * (X + bias)
   * 
   * 2^(X)=e^(ln(2^(X))) 
   * assign ln(2^(X)) = T, Xln2 = T
   * so X = T/ln2
   * (float number <-> integer)
   *  e^(T)        <-> 2^offset * (X+bias) = 2^offset(T/ln2 + bias)
   * because when T/ln2 is not a integer, assgin T/ln2 into E bits of float number will cause accuracy loss
   * so e^(T) <-> 2^offset * (T/ln2 + bias ) - C
   * according to the paper, A Fast, Compact Approximation of the Exponential Function Nicol N. Schraudolph
   * in oder to have the lowest rms relative error
   * C = 2^offset * ln(3/(8ln2) + 1/2)/ln(2)
   * so e^(T) <-> 2^offset * (T/ln2 + (bias- (ln(3/(8ln2) + 1/2)/ln(2))) 
   * 
   * for f16  e^(T) <-> (1<<10)*(1.4426950409T+15-0.0579848147) = 1477.31972187T+15300.623549747
   * for bf16 e^(T) <-> (1<<7)*(1.4426950409T+127-0.0579848147) = 184.664965235T+16248.577943718
   * for f32  e^(T) <-> (1<<23)*(1.4426950409T+127-0.0579848147) = 12102203.161654067 + 1064866804.119529062
   */

  uC.ui = 0x4b38aa3b;
  uD.ui = 0x4e7de250;

  roundingMode = softfloat_round_near_maxMag;
  exact = false;
  uA.ui = f32_to_ui32( f32_add( f32_mul( uC.f, a), uD.f), roundingMode, exact );

  return uA.f;
}