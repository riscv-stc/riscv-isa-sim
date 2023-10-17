#include "fp16am.h"

//-------------------------------------------------------------------------------
//
// ABSTRACT: Floating-point two-operand Adder
//           Computes the addition of two FP numbers. 
//           The format of the FP numbers is defined by the number of bits 
//           in the significand (sig_width) and the number of bits in the 
//           exponent (exp_width).
//           The total number of bits in the FP number is sig_width+exp_width
//           since the sign bit takes the place of the MS bits in the significand
//           which is always 1 (unless the number is a denormal; a condition 
//           that can be detected testing the exponent value).
//           The output is a FP number and status flags with information about
//           special number representations and exceptions. 
//              parameters      valid values 
//              ==========      ============
//              sig_width       significand size,  2 to 253 bits
//              exp_width       exponent size,     3 to 31 bits
//              ieee_compliance 0 or 1
//
//              Input ports     Size & Description
//              ===========     ==================
//              a               (sig_width + exp_width + 1)-bits
//                              Floating-point Number Input
//              b               (sig_width + exp_width + 1)-bits
//                              Floating-point Number Input
//              rnd             3 bits
//                              rounding mode
//
//              Output ports    Size & Description
//              ===========     ==================
//              z               (sig_width + exp_width + 1) bits
//                              Floating-point Number result
//              status          byte
//                              info about FP results
//
//
// MODIFIED:
//
//-------------------------------------------------------------------------------

//typedef unsigned BIT16                          BIT16;

static BIT16 func_RND_eval(BIT16 RND, BIT16 Sign, BIT16 l, BIT16 r, BIT16 STK)
{ // l:guard_bit,r:round_bit,STK:sticky
  BIT16 RND_eval; // 4bits
  RND_eval = 0;
  RND_eval = RND_eval | ((r | STK) & 0x1) << 1;
  switch (RND)
  {
    case RND_NEAREST_EVEN:
      RND_eval = RND_eval | 1<<2 | ((r&(l|STK))&0x1);
    break;
    case RND_TO_ZERO:
      RND_eval = RND_eval;
    break;
    case RND_UP:
      RND_eval = RND_eval | (~Sign)<<3 | (~Sign)<<2 | ((~Sign&(r|STK))&0x1);
    break;
    case RND_DOWN:
      RND_eval = RND_eval | (Sign)<<3 | (Sign)<<2 | ((Sign&(r|STK))&0x1);
    break;
    case RND_NEAREST_UP:
      RND_eval = RND_eval | 1<<2 | (r&0x1);
    break;
    case RND_FROM_ZERO:
      RND_eval = RND_eval | 1<<3 | 1<<2 | ((r&STK)&0x1);
    break;
    default:
      printf("Unknown rounding mode!\n");
  } //printf ("sw:rnd=%x,sign=%x,guard=%x,rounb=%x,stick=%x\n",RND_eval,Sign,l ,r,STK);
  return RND_eval;
}

BIT16 DW_fp_add (BIT16 a,BIT16 b,BIT16 rnd)
{
  BIT16 z;
  //BIT16 exp_width = 5;
  //BIT16 sig_width = 10;
  BIT16 ieee_compliance = 1;
  BIT16 status_int;
  BIT16 z_temp,Large,Small;
  BIT16 swap,subtract,STK;
  BIT16 E_Large,E_Small,E_Diff; // Exponent
  BIT16 F_Large,F_Small;	// Fraction
  BIT16 E_Comp;			// The biggest possible exponent
  BIT16 M_Large,M_Small,M_Z;	// Mantissa
  BIT16 RND_val;		//
  BIT16 NaNFp;
  BIT16 b_int;
  BIT16 Denormal_Large,Denormal_Small;
  
  z_temp=0;Large=0;Small=0;
  M_Large=0;M_Small=0;M_Z=0;
  NaNFp = 0x1f << 10 | 1;
  status_int = 0;
  b_int = b;

  BIT16 a_s,b_s;
  a_s  = (a>>15) & 0x1;
  b_s  = (b>>15) & 0x1;
  subtract = (a_s ^ b_s) &0x1;
  BIT16 a_t,b_t;
  a_t  = a&0x7fff;
  b_t  = b&0x7fff;
  swap = a_t < b_t;
  Large = swap ? b_int : a;
  Small = swap ? a : b_int;
  E_Large = (Large&0x7fff) >> 10;
  E_Small = (Small&0x7fff) >> 10;
  F_Large = Large&0x3ff;
  F_Small = Small&0x3ff;

  if ((((E_Large == ((((1 << (exp_width-1)) - 1) * 2) + 1)) && (F_Large != 0)) ||
      ((E_Small == ((((1 << (exp_width-1)) - 1) * 2) + 1)) && (F_Large != 0))) && ieee_compliance == 1)
  { // NaN input
    z_temp = NaNFp;
    status_int = status_int | 1<<2;
  } else if (E_Large == ((((1 << (exp_width-1)) - 1) * 2) + 1) && (F_Large == 0 || ieee_compliance == 0))
  { // Infinity Input
    z_temp = Large&0xfc00;
    status_int = status_int | 1<<1;
    if ( (E_Small == ((((1 << (exp_width-1)) - 1) * 2) + 1)) && (F_Large == 0 || ieee_compliance == 0) && (subtract == 1) )
    { // Watch out for Inf-Inf
      status_int = status_int | 1<<2;
      status_int = status_int & 0xfffd;
      z_temp = NaNFp; 
    }
  } else if (E_Small == 0 && ((ieee_compliance == 0) || (F_Small == 0)))
  { // Zero Input
    z_temp = Large;
    if (E_Large == 0 && ((ieee_compliance == 0) || (F_Large == 0)))
    { // watch out for 0-0
      if (E_Large == 0 && ((ieee_compliance == 0) || (F_Large == 0)))
      {
        status_int = status_int | 1;
        z_temp = 0;
        if (subtract)
          if (rnd == RND_DOWN) 
            z_temp = 1<<(exp_width + sig_width) | z_temp;
          else
            z_temp = z_temp & 0x7fff;
        else 
          z_temp = a_s<<(exp_width + sig_width) | z_temp;
      }
    }
  } else 
  { // Normal Inputs
    if ((E_Large == 0) && (F_Large != 0))
    { // detect the denormal input
      M_Large = F_Large << 3;
      Denormal_Large = 1;
    } else
    {
      M_Large = 1<<13 | F_Large << 3 ;
      Denormal_Large = 0;
    }

    if ((E_Small == 0) && (F_Small != 0))
    {
      M_Small = F_Small<<3;
      Denormal_Small = 1;
    } else
    {
      M_Small = 1<<13 | F_Small << 3 ;
      Denormal_Small = 0;
    }
    //printf("E_S=%x,F_S=%x,M_S=%x\n",E_Small,F_Small,M_Small);
    // one of the in is a denormal, compensate
    if ((Denormal_Large ^ Denormal_Small)&0x1)
      E_Diff = E_Large - E_Small - 1;
    else
      E_Diff = E_Large - E_Small;
    //printf("E_L=%x,E_S=%x,E_Diff=%x,M_S=%x\n",E_Large,E_Small,E_Diff,M_Small);
    STK = 0;
    while ( (M_Small != 0) && (E_Diff != 0) )
    {
      STK = (M_Small & 0x1) | STK;
      M_Small = M_Small >> 1;
      E_Diff = E_Diff - 1;
    }
    M_Small = M_Small | STK;

      //printf("M_L=%x,F_S=%x,M_S=%x,M_Z=%x\n",M_Large,F_Small,M_Small,M_Z);
    if (subtract == 0) 
      M_Z = M_Large + M_Small;
    else
      M_Z = M_Large - M_Small;
    // Post Process
    E_Comp = E_Large & 0x1f;
    if (M_Z == 0)
    { // Exact 0 special case
      status_int = status_int | 0x1;
      z_temp = 0;
      if (rnd == RND_DOWN)
        z_temp = z_temp | (1<<15);
    } else 
    { // Normalize the M for overflow
      if (((M_Z>>14)&0x1) == 1)
      {
        E_Comp = E_Comp + 1;
        STK = M_Z & 0x1;
        M_Z = M_Z >> 1;
        M_Z = M_Z | STK;
      }

      while ((((M_Z>>13)&0x1) == 0) && (E_Comp > 1))
      {
        E_Comp = E_Comp -1;
        M_Z = M_Z << 1;
      }

      if ((((M_Z>>14)&0x1) == 0)&&(((M_Z>>13)&0x1) == 0))
        if (ieee_compliance == 1)
        {
          z_temp = ((Large>>15)&0x1)<<15 | ((M_Z>>3)&0x3ff);
          status_int = status_int & 0xfff7;
          if ((STK == 1) || ((M_Z&0x7)!=0))
            status_int = status_int | 1<<5;
          if (((M_Z>>3)&0x3ff)==0)
            status_int = status_int | 1;
        } else
        { // when denormal is not used
          // TBD
        }
      else
      { // Round M_Z according to the rnd
        RND_val = func_RND_eval(rnd, 
          (Large>>15)&0x1,
          (M_Z>>3)&0x1,
          (M_Z>>2)&0x1,
          (((M_Z>>1)&0x1)|(M_Z&0x1))
        );
        if ((RND_val&0x1)==1)
          M_Z = M_Z + (1<<3);
        // Normalize M for overflow 
        if (((M_Z>>14)&0x1)==1)
        {
          E_Comp = E_Comp + 1;
          M_Z = M_Z >> 1;
        }
        // huge
        if (E_Comp >= ((((1 << (exp_width-1)) - 1) * 2) + 1))
        {
          status_int = status_int | 1<<4 | 1<<5;
          if (((RND_val>>2)&0x1)==1)
          { // Infinity
            M_Z = M_Z & 0xe007;
            E_Comp = ((((1 << (exp_width-1)) - 1) * 2) + 1);
            status_int = status_int | 1<<1 ;
          } else
          { // MaxNorm
            E_Comp = ((((1 << (exp_width-1)) - 1) * 2) + 1) - 1;
            M_Z = M_Z | 0x1ff8;
          }
        } else 
        { // Tiny or Denormal
          if (E_Comp <= 0) E_Comp = 0 + 1;
        }
        // Normal continues
        status_int = status_int | ((RND_val>>1)<<5);
        // Reconstruct
        z_temp = ((Large >> 15) & 0x1) << 15
               | (E_Comp & 0x1f) << 10
               | ((M_Z>>3) & 0x3ff);
      }
    }   
  }
  //printf("Large=%x E_Comp=%x M_Z=%x\n",Large,E_Comp,M_Z);
  z = z_temp;
  return z;
}

BIT16 func_CS16FA (BIT16 a, BIT16 b)
{
  BIT16 z;
  z = DW_fp_add(a,b,4);

  //printf("hex:%x+%x=%x\n",a,b,z);
  return z;
}
BIT16 double2BIT16 (double iif)
{
  BIT16 ob;
  BIT16 s,e,m;
  BIT16 expBitNum = 5;
  BIT16 baseBitNum = 15 - expBitNum;
  int   maxExp = pow(2,expBitNum);
  int   maxBase= pow(2,baseBitNum);
  int   biasExp= maxExp/2 - 1;
  s = iif < 0;
  double thd1,thd2;
  thd1 = (maxBase-1)*1.0/maxBase * pow(2,(1 - biasExp));
  thd2 = 1.0/maxBase * pow(2,(1 - biasExp));
  double x;
  x = s ? -iif : iif;
  BIT16 indA,indB;
  indA = x < thd2/2;
  indB = x > thd2/2;
  if (indA)
  {
    e = 0;
    m = 0;
    s = 0;
  }
  if (indB)
  {
    double xl;
    xl = log2(x);
    e = biasExp + floor(xl);

    if (e > (maxExp - 1)) printf("[double2BIT16]Error: out of e range\n");
  }
  BIT16 ind1,ind2;
  ind1 = e <= 0;
  ind2 = e > 0;
  if (ind1)
  {
    e = 0;
    m = round(x * pow(2,(biasExp - 1)) * maxBase);
  }
  if (ind2)
  {
    double xr;
    xr = x / pow(2,(e - biasExp)) - 1;
    m = round(xr * maxBase);
  }
  
  ob = (s&0x1) << 15 | (((e&0x1f) << 10) + m);
  return ob;
}
double BIT16double (BIT16 ib)
{
  double of;
  BIT16 expBitNum = 5;
  BIT16 baseBitNum = 15 - expBitNum;
  int   maxExp = pow(2,expBitNum);
  int   maxBase= pow(2,baseBitNum);
  int   biasExp= maxExp/2 - 1;
  BIT16 s,e,m;
  s = (ib >> 15) & 0x1;
  e = (ib >> 10) & 0x1f;
  m = ib & 0x3ff;
  BIT16 ind1,ind2;
  ind1 = e ==0;
  ind2 = e > 0;
  of = 0;
  if (ind1)
    of = pow((-1),s) * m*1.0/maxBase * pow(2,(1 - biasExp));
  if (ind2)
    of = pow((-1),s) * (1 + m*1.0/maxBase) * pow(2,(e - biasExp));

  return of;
}
double f16a (double i0f, double i1f)
{
  double of;
  BIT16 i0b,i1b,ob;
  i0b = double2BIT16 (i0f);
  i1b = double2BIT16 (i1f);
  ob  = func_CS16FA  (i0b,i1b);
  of  = BIT16double  (ob);
  //printf("float:%lf+%lf=%lf(hex:%x+%x=%x)\n",i0f,i1f,of,i0b,i1b,ob);
  return of;
}
