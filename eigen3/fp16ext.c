#include "fp16am.h"

#define fp16_add(a, b) func_CS16FA(a, b)
#define fp16_mul(a, b) func_CS16FM(a, b)
#define fp16_sub(a, b) func_CS16FA(a, (b) ^ 0x8000)

BIT16 fp16_exp(BIT16 x) 
{
  // get exp part
  int e = (int)((x >> 10) & 0x1f) - 14;
  int sign_x  =  (x>>15)  & 0x1;
  int exp_x   =  (x>>10) & 0x1f;
  int frac_x  =  (x>>0) & 0x3ff;
  
  if (exp_x==0x1f && frac_x!=0) {
      return 0x7c01; //nan
  }
  else if (exp_x==0x1f && frac_x==0) { //inf
    if(sign_x==1)  
        return 0;
    else  
        return 0x7c00;
  }
  else if (exp_x==0) { //0 or subnormal
      return 0x3c00;  
  }

  if (e > 4) return (x & 0x8000) ? 0 : 0x7c00 /* +inf */;
  if (e > 0) {
    // set exp part to 0
    x &= 0x83ff;
    x |= 0x3800;
  }

  BIT16 c1 = 0x3c00; //1.0
  BIT16 c2 = 0x3800; //1.0/2
  BIT16 c3 = 0x3166; //1.0/6
  BIT16 c4 = 0x2955; //1.0/24
  BIT16 r;

  r = fp16_add(fp16_mul(x, c4), c3);
  r = fp16_add(fp16_mul(r, x), c2);
  r = fp16_add(fp16_mul(r, x), c1);
  r = fp16_add(fp16_mul(r, x), c1);

  if (e == 4) {
    r = fp16_mul(r, r);
    r = fp16_mul(r, r);
    r = fp16_mul(r, r);
    r = fp16_mul(r, r);
  } else if (e == 3) {
    r = fp16_mul(r, r);
    r = fp16_mul(r, r);
    r = fp16_mul(r, r);
  } else if (e == 2) {
    r = fp16_mul(r, r);
    r = fp16_mul(r, r);
  } else if (e == 1) {
    r = fp16_mul(r, r);
  }

  return r;
}

//count 1/(x ^ 0.5) for fp16
BIT16 fp16_rsqrt(BIT16 x) 
{
  BIT16 x2, y;
  BIT16 threehalfs = 0x3e00; /* 1.5 in fp16 */
  BIT16 half_fp16 = 0x3800; /* 0.5 in fp16 */

  x2 = fp16_mul(x, half_fp16);
  y = 0x59ba - (x >> 1);
  y = fp16_mul(y, fp16_sub(threehalfs, fp16_mul(fp16_mul(x2, y), y)));
  // y = fp16_mul(y, fp16_sub(threehalfs, fp16_mul(fp16_mul(x2, y), y)));
  return y;
}

//count 1/x for fp16
BIT16 fp16_recip(BIT16 x)
{
  BIT16 value;
  BIT16 t;
  int sign_x  =  (x>>15)  & 0x1;
  int exp_x   =  (x>>10) & 0x1f;
  int frac_x  =  (x>>0) & 0x3ff;

  if (exp_x == 0x1f && frac_x != 0) { //nan
      if(sign_x==1) 
          return 0xfc01;
      return 0x7c01; 
  }
  else if (exp_x == 0x1f && frac_x == 0) { //inf
    if (sign_x == 1)  
        return 0x8000 ;
    else
        return 0x0;
  }
  else if (exp_x == 0 && frac_x != 0) { //subnormal
    if(sign_x == 1)    
        return 0xfc00;
    else 
        return 0x7c00;  
  }
  else if (exp_x == 0 && frac_x == 0) { //0
    if (sign_x==1)    
        return 0xfc01;
    else  
        return 0x7c01;  
  }

  t = x & 0x7fff;  
  value = fp16_rsqrt(t);
  t = fp16_mul(value, value);
  return t | (sign_x << 15);
}

//count x ^ 0.5 for fp16
BIT16 fp16_sqrt(BIT16 x)
{
  BIT16 value;
  // get exp part
  int sign_x  =  (x>>15)  & 0x1;
  int exp_x   =  (x>>10) & 0x1f;
  int frac_x   =  (x>>0) & 0x3ff;
  
  if (sign_x == 1) { //sqrt(-) is nan
     return 0x7c01;
  }

  if (exp_x == 0x1f && frac_x != 0) { //nan
      return 0x7c01; 
  }
  else if (exp_x == 0x1f && frac_x == 0){ //inf
      return 0x7c00;
  }
  else if (exp_x == 0) { //0 or subnormal
     return 0;  
  }

  value = fp16_rsqrt(x);
  return fp16_mul(x, value);
}




