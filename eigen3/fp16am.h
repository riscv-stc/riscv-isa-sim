#ifndef FP16AM_H
#define FP16AM_H

#include <stdio.h>
#include <math.h>

typedef unsigned short                          BIT16;
typedef int                                     BIT32;

#define RND_NEAREST_EVEN 0 //000 RNE Round to Nearest, ties to Even
#define RND_TO_ZERO      1 //001 RTZ Round towards Zero
#define RND_DOWN         2 //010 RDN Round Down (towards �?)
#define RND_UP           3 //011 RUP Round Up (towards +1)
#define RND_NEAREST_UP   4 //100 RMM Round to Nearest, ties to Max Magnitude
#define RND_FROM_ZERO    5 //101 ??? Round away from zero

#define sig_width 10
#define exp_width 5
#define Mwidth (2 * sig_width + 3) // 23
#define Movf   (Mwidth - 1)        // 22
//#define L      (Movf - 1 - sig_width) //11
//#define R      (L - 1)                //10
#define RND_Width  4
#define RND_Inc  0
#define RND_Inexact  1
#define RND_HugeInfinity  2
#define RND_TinyminNorm  3
#define log_awidth ((sig_width + 1>65536)?((sig_width + 1>16777216)?((sig_width + 1>268435456)?((sig_width + 1>536870912)?30:29):((sig_width + 1>67108864)?((sig_width + 1>134217728)?28:27):((sig_width + 1>33554432)?26:25))):((sig_width + 1>1048576)?((sig_width + 1>4194304)?((sig_width + 1>8388608)?24:23):((sig_width + 1>2097152)?22:21)):((sig_width + 1>262144)?((sig_width + 1>524288)?20:19):((sig_width + 1>131072)?18:17)))):((sig_width + 1>256)?((sig_width + 1>4096)?((sig_width + 1>16384)?((sig_width + 1>32768)?16:15):((sig_width + 1>8192)?14:13)):((sig_width + 1>1024)?((sig_width + 1>2048)?12:11):((sig_width + 1>512)?10:9))):((sig_width + 1>16)?((sig_width + 1>64)?((sig_width + 1>128)?8:7):((sig_width + 1>32)?6:5)):((sig_width + 1>4)?((sig_width + 1>8)?4:3):((sig_width + 1>2)?2:1)))))
#define ez_msb ((exp_width >= log_awidth) ? exp_width + 1 : log_awidth + 1)

//#ifdef __cplusplus
//extern "C" {
//#endif

BIT16 func_CS16FM (BIT16 a, BIT16 b);
BIT16 func_CS16FA (BIT16 a, BIT16 b);
//BIT16 func_RND_eval(BIT16 RND, BIT16 Sign, BIT16 l, BIT16 r, BIT16 STK);

//#ifdef __cplusplus
//}
//#endif

#endif

