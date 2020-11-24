#include "fp16am.h"

#define fp16_add(a, b) func_CS16FA(a, b)
#define fp16_mul(a, b) func_CS16FM(a, b)
#define fp16_sub(a, b) func_CS16FA(a, (b) ^ 0x8000)
int fp16tofp32(int fp_data_in);
int fp32tofp16(int fp_data);

int fp16_to_int(BIT16 x)
{
    float result;
    *(int *)&result = fp16tofp32(x);
    return (int)result;
}

int fp16_to_int_floor(BIT16 x)
{
    float result;
    *(int *)&result = fp16tofp32(x);
    return floorf(result);
}

BIT16 int_to_fp16(int x)
{
    int f;
    *(float*)&f = (float)x;
    return fp32tofp16(f);
}

BIT16 fp16_exp(BIT16 x) {
  int sign_x = (x>>15) & 0x1;
  int exp_x  = (x>>10) & 0x1f;
  int frac_x  = (x&0x3ff);
  int frac_x_a  = (0x1<<10) | (x&0x3ff);

  if(exp_x==0x1f && frac_x!=0){
       return 0x7c01; //nan
  }
  else if(exp_x==0x1f && frac_x==0){ //inf
     if(sign_x==1)  return 0;
     else  return 0x7c00;
  }
  else if(exp_x==0 && frac_x==0){ //0
       return 0x3c00;
  }
  
  int index = fp16_to_int_floor(x) + 12;
  if (index < 0) return 0;
  if (index > 23) return 0x7c00;
  int shifts[] = {-17, -16, -14, -12, -11, -10, -8, -7, 
      -5, -4, -2, -1, 1, 2, 3, 5, 6, 8, 9, 11, 12, 14, 15, 17};
  int shift = shifts[index];

#define PRECOMPUTED_SUBS 1
#if PRECOMPUTED_SUBS
  BIT16 subs[] = {0xc9e5, 0xc98c, 0xc8da, 0xc829, 0xc7a0, 0xc6ef, 
      0xc58c, 0xc4da, 0xc2ef, 0xc18c, 0xbd8c, 0xb98c, 0x398c, 0x3d8c, 0x4029, 
      0x42ef, 0x4429, 0x458c, 0x463e, 0x47a0, 0x4829, 0x48da, 0x4933, 0x49e5};
  BIT16 sub = subs[index];
#else
  BIT16 sub = fp16_mul(0x398c, int_to_fp16(shift));
#endif
   
  x = fp16_sub(x, sub);
  // taylor expansion at x = 0
  BIT16 c0 = 0x3c00;
  BIT16 c1 = 0x3c00; //1.0
  BIT16 c2 = 0x3800; //1.0/2
  BIT16 c3 = 0x3166; //1.0/6
  BIT16 c4 = 0x2955; //1.0/24
  BIT16 r;
  r = fp16_add(fp16_mul(x, c4), c3);
  r = fp16_add(fp16_mul(r, x), c2);
  r = fp16_add(fp16_mul(r, x), c1);
  r = fp16_add(fp16_mul(r, x), c0);

#define OPTIONAL_FIXUP 1
#if OPTIONAL_FIXUP
  int adds[] = {0, 10, 2, -4, -1, -3, -3, 1, -2, -1, 0, 0, 0, 1, 1, 2, 2, 2, 6, 1, 4, -1, 2, 7};
  r += adds[index];
#endif

  // get exp part
  int e = (int)((r >> 10) & 0x1f);
  e += shift;
  if (e < 0) return 0;
  if (e > 30) return 0x7c00;
  r &= 0x83ff;
  r |= (e << 10);
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
  else if ((exp_x == 0) && (frac_x == 0)) { //0
     return 0;  
  }

  value = fp16_rsqrt(x);
  return fp16_mul(x, value);
}




