#include <stdio.h>
#include <math.h>
//-------------------------------------------------------------------------------
//
// ABSTRACT: Floating-point two-operand Multiplier 
//              DW_fp_mult calculates the floating-point multiplication
//              while supporting six rounding modes, including four IEEE
//              standard rounding modes.
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

typedef unsigned short                          BIT16;
#define RND_NEAREST_EVEN 0 //000 RNE Round to Nearest, ties to Even
#define RND_TO_ZERO      1 //001 RTZ Round towards Zero
#define RND_UP           2 //010 RDN Round Down (towards âˆ?)
#define RND_DOWN         3 //011 RUP Round Up (towards +1)
#define RND_NEAREST_UP   4 //100 RMM Round to Nearest, ties to Max Magnitude
#define RND_FROM_ZERO    5 //101 ??? Round away from zero

#define sig_width 10
#define exp_width 5
#define Mwidth (2 * sig_width + 3) // 23
#define Movf   (Mwidth - 1)        // 22
#define L      (Movf - 1 - sig_width) //11
#define R      (L - 1)                //10
#define RND_Width  4
#define RND_Inc  0
#define RND_Inexact  1
#define RND_HugeInfinity  2
#define RND_TinyminNorm  3
#define log_awidth ((sig_width + 1>65536)?((sig_width + 1>16777216)?((sig_width + 1>268435456)?((sig_width + 1>536870912)?30:29):((sig_width + 1>67108864)?((sig_width + 1>134217728)?28:27):((sig_width + 1>33554432)?26:25))):((sig_width + 1>1048576)?((sig_width + 1>4194304)?((sig_width + 1>8388608)?24:23):((sig_width + 1>2097152)?22:21)):((sig_width + 1>262144)?((sig_width + 1>524288)?20:19):((sig_width + 1>131072)?18:17)))):((sig_width + 1>256)?((sig_width + 1>4096)?((sig_width + 1>16384)?((sig_width + 1>32768)?16:15):((sig_width + 1>8192)?14:13)):((sig_width + 1>1024)?((sig_width + 1>2048)?12:11):((sig_width + 1>512)?10:9))):((sig_width + 1>16)?((sig_width + 1>64)?((sig_width + 1>128)?8:7):((sig_width + 1>32)?6:5)):((sig_width + 1>4)?((sig_width + 1>8)?4:3):((sig_width + 1>2)?2:1)))))
#define ez_msb ((exp_width >= log_awidth) ? exp_width + 1 : log_awidth + 1)


BIT16 func_RND_eval(BIT16 RND, BIT16 Sign, BIT16 l, BIT16 r, BIT16 STK)
{ // l:guard_bit,r:round_bit,STK:sticky
  BIT16 RND_eval; // 4bits
  RND_eval = 0;
  RND_eval = RND_eval | ((r | STK) & 0x1) << 1;
  switch (RND)
  {
    case RND_NEAREST_EVEN:
      RND_eval = RND_eval | 1<<2 | (r&(l|STK))&0x1;
    break;
    case RND_TO_ZERO:
      RND_eval = RND_eval;
    break;
    case RND_UP:
      RND_eval = RND_eval | (~Sign)<<3 | (~Sign)<<2 | (~Sign&(r|STK))&0x1;
    break;
    case RND_DOWN:
      RND_eval = RND_eval | (Sign)<<3 | (Sign)<<2 | (Sign&(r|STK))&0x1;
    break;
    case RND_NEAREST_UP:
      RND_eval = RND_eval | 1<<2 | r&0x1;
    break;
    case RND_FROM_ZERO:
      RND_eval = RND_eval | 1<<3 | 1<<2 | (r&STK)&0x1;
    break;
    default:
      printf("Unknown rounding mode!\n");
  } printf ("sw:rnd=%x,sign=%x,guard=%x,rounb=%x,stick=%x\n",RND_eval,Sign,l ,r,STK);
  return RND_eval;
}

BIT16 DW_fp_mult (BIT16 a,BIT16 b,BIT16 rnd)
{
  BIT16 z;
  //BIT16 exp_width = 5;
  //BIT16 sig_width = 10;
  BIT16 ieee_compliance = 1;
  BIT16 z_reg;
  BIT16 EA,EB;
  signed int EZ,Range_Check,SH_Shift,EZ_Shift;
  BIT16 MA,MB,TMP_MA,TMP_MB;
  int MZ;
  BIT16 STK,SIGN,RND_val,status_reg;
  BIT16 MaxEXP_A,MaxEXP_B,InfSIG_A,InfSIG_B,Zero_A,Zero_B,Denorm_A,Denorm_B;
  BIT16 LZ_INA,LZ_INB,LZ_IN,SIGA,SIGB,NaN_Reg,Inf_Reg;
  BIT16 MZ_Movf1,EZ_Zero,STK_PRE,STK_EXT,NaN_Sig,Inf_Sig,STK_CHECK,minnorm_case;
  BIT16 i;

  SIGN = ((a>>15) ^ (b>>15)) &0x1;
  EA = (a>>10)&0x1f;
  EB = (b>>10)&0x1f;
  SIGA = a & 0x3ff;
  SIGB = b & 0x3ff;
  status_reg = 0;
  LZ_INA = 0;
  LZ_INB = 0;
  LZ_IN = 0;
  STK_EXT = 0;

  MaxEXP_A = (EA == ((((1 << (exp_width-1)) - 1) * 2) + 1));
  MaxEXP_B = (EB == ((((1 << (exp_width-1)) - 1) * 2) + 1));
  InfSIG_A = (SIGA == 0);
  InfSIG_B = (SIGB == 0);

  // Zero and Denormal
  if (ieee_compliance)
  {
    Zero_A = (EA == 0 ) & (SIGA == 0);
    Zero_B = (EB == 0 ) & (SIGB == 0);
    Denorm_A = (EA == 0 ) & (SIGA != 0);
    Denorm_B = (EB == 0 ) & (SIGB != 0);
    // IEEE Standard
    NaN_Sig = 1;
    Inf_Sig = 0;
    NaN_Reg = 0x1f<<10 | NaN_Sig; //{1'b0, {(exp_width){1'b1}}, NaN_Sig};
    Inf_Reg = SIGN<<15 | 0x1f<<10 | Inf_Sig; //{SIGN, {(exp_width){1'b1}}, Inf_Sig};

    if (Denorm_A) {
      MA = a&0x3ff; //{1'b0, a[(sig_width - 1):0]};
    }
    else {
      MA = 1<<10 | (a&0x3ff); //{1'b1, a[(sig_width - 1):0]};
    }

    if (Denorm_B) {
      MB = b&0x3ff; //{1'b0, b[(sig_width - 1):0]};
    }
    else {
      MB = 1<<10 | (b&0x3ff); //{1'b1, b[(sig_width - 1):0]};
    }
  } else 
  {
    //TBD
  }
  // Infinity & NaN input
  if (ieee_compliance && ((MaxEXP_A && !InfSIG_A) || (MaxEXP_B && !InfSIG_B)))
  {
    status_reg = status_reg | 1<<2;
    z_reg = NaN_Reg;
  } else if ( (MaxEXP_A) || (MaxEXP_B) )
  {
    if (ieee_compliance == 0)
      status_reg = status_reg | 1<<1;
    // 0*Inf = NaN
    if ( Zero_A || Zero_B )
    {
      status_reg = status_reg | 1<<2;
      z_reg = NaN_Reg;
    } else
    { // Infinity
      status_reg = status_reg | 1<<1;
      z_reg = Inf_Reg;
    }
  } else if (Zero_A || Zero_B)
  { // Zero Input
    status_reg = status_reg | 1;
    z_reg = 0;
    z_reg = z_reg | SIGN<<15;
  } else
  { // Normal & Denormal
    // Denormal check
    TMP_MA = MA;
    if (Denorm_A) 
    {
      while( ((TMP_MA >> 10) & 0x1) != 1)
      {
        TMP_MA = TMP_MA << 1;
        LZ_INA = LZ_INA + 1;
      }
    }

    TMP_MB = MB;
    if (Denorm_B) 
    {
      while( ((TMP_MB >> 10) & 0x1) != 1)
      {
        TMP_MB = TMP_MB << 1;
        LZ_INB = LZ_INB + 1;
      }
    }

    LZ_IN = LZ_INA + LZ_INB;
    EZ = EA + EB - LZ_IN + Denorm_A + Denorm_B;
    MZ = MA * MB;
    printf("MA*MB=%x\n",MZ);
    // denormal multiplication
    if (ieee_compliance)
      MZ = MZ << LZ_IN;
    printf("MZ<<%x=%x\n",LZ_IN,MZ);
    // After the computation, left justify the Mantissa to `Movf-1 bit.
    // Note that the normalized Mantissa after computation is in `Movf-2 bit,
    // and now we normalize it to `Movf-1 bit.
    MZ_Movf1 = (MZ>>(Movf-1)) & 0x1;
    int MZ_TEMP = MZ; BIT16 MZ_MSB=MZ>> Movf;
    if (MZ_Movf1 == 1)
    {
      EZ = EZ + 1;
      minnorm_case = 0;
    } else 
    {
      MZ = MZ << 1;
      minnorm_case = (EZ - ((1 << (exp_width-1)) - 1) == 0) ? 1 : 0;
    }
    // Denormal support
    if (ieee_compliance)
    {
      Range_Check = EA + EB + Denorm_A + Denorm_B + MZ_Movf1 - ((1<<(exp_width-1))-1) - LZ_IN -1;
      EZ_Shift = -Range_Check;
      printf("Range_Ch=%x,EZ_Shift=%x\n",Range_Check,EZ_Shift);
      if (EZ_Shift >= 0)
      {
        for (i = 0; i< EZ_Shift; i=i+1)
        {
          //{MZ, STK_CHECK} = {MZ, 1'b0} >> 1;
          STK_CHECK = MZ&0x1;
          MZ = MZ>>1;
          STK_EXT = STK_EXT | STK_CHECK;
        }
      }
      printf("after shift,STK_CH=%x,STK_EXT=%x,MZ=%x\n",STK_CHECK,STK_EXT,MZ);
    }
    if (((MZ&0x3ff)==0)&&(STK_EXT==0)) 
      STK = 0;
    else 
      STK = 1;

    //if (MZ_MSB == 1)
      RND_val = func_RND_eval(rnd,SIGN,(MZ>>L)&0x1,(MZ>>R)&0x1,STK);
      //RND_val = func_RND_eval(rnd,SIGN,(MZ_TEMP>>L)&0x1,(MZ_TEMP>>R)&0x1,STK);
    //else
    //  RND_val = func_RND_eval(rnd,SIGN,(MZ>>(L-1))&0x1,(MZ>>(R-1))&0x1,STK);
    //  RND_val = func_RND_eval(rnd,SIGN,(MZ_TEMP>>(L-1))&0x1,(MZ_TEMP>>(R-1))&0x1,STK);
    printf("before Rnd add, MZ=%x,MZ_TEMP=%x\n",MZ,MZ_TEMP);
    //printf("sw:rnd=%x)\n",RND_val);
    // Round Addition
    if ((RND_val&0x1)==1)
      MZ = MZ + (1<<L);
      //MZ = MZ + (((MZ>>R)&0x1)<<L);

    // Normalize the M for overflow
    if (((MZ>>Movf)&0x1)==1)
    {
      EZ = EZ + 1;
      MZ = MZ >> 1;
    }
    printf("after Rnd add, EZ=%x,MZ=%x\n",EZ,MZ);
    // Correction of denormal output
    if (ieee_compliance & (EZ <= ((1 << (exp_width-1)) - 1)) & ((MZ>>(Movf - 1))&0x1)) 
      EZ = EZ + 1;
    EZ_Zero = (EZ == ((1 << (exp_width-1)) - 1));
    // Adjust Exponent ((1 << (exp_width-1)) - 1).
    // Force EZ = 0 if underflow occurs becuase of subtracting ((1 << (exp_width-1)) - 1),
    if((((EZ>>ez_msb)&0x1) == 0) & (EZ >= ((1 << (exp_width-1)) - 1))) 
      EZ = EZ - ((1 << (exp_width-1)) - 1);
    else EZ = 0;
    printf("after adjust, EZ=%x,MZ=%x\n",EZ,MZ);
    // Huge
    if (EZ >= ((((1 << (exp_width-1)) - 1) * 2) + 1))
    {
      status_reg = status_reg | 1<<4 | 1<<5;
      if(((RND_val>>RND_HugeInfinity)&0x1)==1)
      { // Infinity
        MZ = (MZ&0xffe00000) | (Inf_Sig & 0x3ff) <<L | MZ & 0x7ff;
        EZ = ((((1 << (exp_width-1)) - 1) * 2) + 1);
        status_reg = status_reg | 1<<1;
      } else
      { // MaxNorm
        EZ = ((((1 << (exp_width-1)) - 1) * 2) + 1) - 1;
        MZ = (MZ&0xffe00000) |  0x3ff <<L | MZ & 0x7ff;
      }
    } else if (EZ == 0 )
    { // Tiny
      status_reg = status_reg | 1<<3;
      if (ieee_compliance == 0)
      {
      } // TBD
      if ((((MZ>>11)&0x3ff)==0)&((EZ&0x1f)==0))
        status_reg = status_reg | 0x1;
    }
    printf("final, EZ=%x,MZ=%x\n",EZ,MZ);
    status_reg = status_reg | ( (RND_val>>1)&0x1
                              | ( ~(Zero_A | Zero_B) 
                                & ((EZ&0x1f) == 0) 
                                & (((MZ>>11)&0x3ff) == 0)
                                )
                              )<<5;
    // Reconstruct
    z_reg = SIGN<<15 | (EZ&0x1f)<<10 | ((MZ>>11)&0x3ff) ;
  }
  z = z_reg;
  return z;
}

BIT16 func_CS16FM (BIT16 a, BIT16 b)
{
  BIT16 z;
  z = DW_fp_mult(a,b, RND_NEAREST_UP);
                      //RND_FROM_ZERO);
  printf("(hex:%x*%x=%x)\n",a,b,z);
  return z;
}
BIT16 double2BIT16 (double iif)
{
  BIT16 ob;
  int s,e,m;
  int expBitNum = 5;
  int baseBitNum = 15 - expBitNum;
  int   maxExp = pow(2,expBitNum);
  int   maxBase= pow(2,baseBitNum);
  int   biasExp= maxExp/2 - 1;
  s = iif < 0;
  double thd1,thd2;
  thd1 = (maxBase-1)*1.0/maxBase * pow(2,(1 - biasExp));
  thd2 = 1.0/maxBase * pow(2,(1 - biasExp));
  double x;
  x = s ? -iif : iif;
  int indA,indB;
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
  int ind1,ind2;
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
    xr = x*1.0 / pow(2,(e - biasExp)) - 1;
    m = round(xr * maxBase);
  }
  
  ob = (s&0x1) << 15 | (((e&0x1f) << 10) + m);
  return ob;
}
double BIT16double (BIT16 ib)
{
  double of;
  int expBitNum = 5;
  int baseBitNum = 15 - expBitNum;
  int   maxExp = pow(2,expBitNum);
  int   maxBase= pow(2,baseBitNum);
  int   biasExp= maxExp/2 - 1;
  int s,e,m;
  s = (ib >> 15) & 0x1;
  e = (ib >> 10) & 0x1f;
  m = ib & 0x3ff;
  int ind1,ind2;
  ind1 = e ==0;
  ind2 = e > 0;
  of = 0;
  if (ind1)
    of = pow((-1),s) * ((m*1.0)/maxBase) * pow(2,(1 - biasExp));
  if (ind2)
    of = pow((-1),s) * (1 + (m*1.0)/maxBase) * pow(2,(e - biasExp));
  printf("e=%d,m=%d\n",e,m);
  printf("x=%lf,y=%lf\n",(1 + (m*1.0)/maxBase),pow(2.0,(e - biasExp)));
  return of;
}
double f16m (double i0f, double i1f)
{
  double of;
  BIT16 i0b,i1b,ob;
  i0b = double2BIT16 (i0f);
  i1b = double2BIT16 (i1f);
  ob  = func_CS16FM  (i0b,i1b);
  of  = BIT16double  (ob);
  //printf("double:%lf*%lf=%lf(hex:%x*%x=%x)\n",i0f,i1f,of,i0b,i1b,ob);
  //printf("dec  : %d * %d = %d\n",i0b,i1b,ob);
  return of;
}
#ifndef DPI_TEST_ON
int main()
{
  double i0f,i1f,of;
  i0f = 128;
  i1f = 0.0013579;
  of  = f16m(i0f,i1f);
  printf("ref:%f * %f = %f\n",i0f,i1f,i0f*i1f);
}
#endif
