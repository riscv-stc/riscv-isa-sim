/*ANDES exchange between fp16 and fp32*/
int fp16tofp32(int fp_data_in){
    int fp_data;
    int fp_out;
    
    int fp_is_0;
    int fp_is_subnormal;
    int fp_is_infinite;
    int fp_is_nan;
    int fp_single_exp_fra;
    int fp_conv_sign;
    int fp_conv_exp;
    int fp_conv_fraction;
    
    fp_data = fp_data_in & 0xffff;
    
    fp_is_0         = (((fp_data>>10)&0x1f) == 0x0) & (((fp_data>>0)&0x3ff) == 0x0);
    fp_is_subnormal = (((fp_data>>10)&0x1f) == 0x0) & (((fp_data>>0)&0x3ff) != 0x0);
    fp_is_infinite  = (((fp_data>>10)&0x1f) == 0x1f) & (((fp_data>>0)&0x3ff) == 0x0);
    fp_is_nan       = (((fp_data>>10)&0x1f) == 0x1f) & (((fp_data>>0)&0x3ff) != 0x0);

    if((fp_data>>9)&0x1)      fp_single_exp_fra = (112<<23) | ((fp_data&0x1ff)<<14);
    else if((fp_data>>8)&0x1) fp_single_exp_fra = (111<<23) | ((fp_data&0xff)<<15);
    else if((fp_data>>7)&0x1) fp_single_exp_fra = (110<<23) | ((fp_data&0x7f)<<16);
    else if((fp_data>>6)&0x1) fp_single_exp_fra = (109<<23) | ((fp_data&0x3f)<<17);
    else if((fp_data>>5)&0x1) fp_single_exp_fra = (108<<23) | ((fp_data&0x1f)<<18);
    else if((fp_data>>4)&0x1) fp_single_exp_fra = (107<<23) | ((fp_data&0xf)<<19);
    else if((fp_data>>3)&0x1) fp_single_exp_fra = (106<<23) | ((fp_data&0x7)<<20);
    else if((fp_data>>2)&0x1) fp_single_exp_fra = (105<<23) | ((fp_data&0x3)<<21);
    else if((fp_data>>1)&0x1) fp_single_exp_fra = (104<<23) | ((fp_data&0x1)<<22);
    else if((fp_data>>0)&0x1) fp_single_exp_fra = (103<<23);
    else                      fp_single_exp_fra = 0;

    if(fp_is_nan)  fp_conv_sign     = 0x0;
    else           fp_conv_sign     = (fp_data>>15) & 0x1;

    if(fp_is_subnormal)        fp_conv_exp = (fp_single_exp_fra>>23) & 0xff;
    else if(fp_is_0)           fp_conv_exp = 0x0;
    else if(fp_is_infinite)    fp_conv_exp = 0xff;
    else if(fp_is_nan)         fp_conv_exp = 0xff;
    else                       fp_conv_exp = ((fp_data>>10)&0x1f) + 112;
    
    if(fp_is_subnormal)        fp_conv_fraction = (fp_single_exp_fra>>0) & 0x7fffff;
    else if(fp_is_0)           fp_conv_fraction = 0x0;
    else if(fp_is_infinite)    fp_conv_fraction = 0x0;
    else if(fp_is_nan)         fp_conv_fraction = 0x400000;
    else                       fp_conv_fraction = ((fp_data>>0)&0x3ff)<<13;

    fp_out = ((fp_conv_sign&0x1)<<31) | ((fp_conv_exp&0xff)<<23) | (fp_conv_fraction&0x7fffff);

    return fp_out;
}

int fp32tofp16(int fp_data){
    int fp_out;
    int fp_is_subnormal;
    int fp_is_infinite;
    int fp_is_zero;
    int fp_is_nan;
    int fp_is_of;
    int fp_is_uf;
    int fp_inexact_subnormal_value;
    int fp_exact_subnormal_value;
    int fp_subnormal_fraction;
    int fp_conv_sign;
    int fp_conv_exp_truncate;
    int fp_conv_exp;
    int fp_conv_fraction;

    fp_is_subnormal           = (((fp_data>>23)&0xff) == 0x0) & (fp_data&0x7fffff != 0x0);
    fp_is_infinite            = (((fp_data>>23)&0xff) == 0xff) & (fp_data&0x7fffff == 0x0);
    fp_is_zero                = ((fp_data&0x7fffffff) == 0x0);
    fp_is_nan                 = (((fp_data>>23)&0xff) == 0xff) & (fp_data&0x7fffff != 0x0);
    fp_is_of                  = (((fp_data>>23)&0xff) > 142) & ~fp_is_infinite & ~fp_is_nan;
    fp_exact_subnormal_value  = (((fp_data>>23)&0xff) == 112) & ((fp_data&0x3fff) == 0)
                                | (((fp_data>>23)&0xff) == 111) & ((fp_data&0x7fff) == 0)
                                | (((fp_data>>23)&0xff) == 110) & ((fp_data&0xffff) == 0)
                                | (((fp_data>>23)&0xff) == 109) & ((fp_data&0x1ffff) == 0)
                                | (((fp_data>>23)&0xff) == 108) & ((fp_data&0x3ffff) == 0)
                                | (((fp_data>>23)&0xff) == 107) & ((fp_data&0x7ffff) == 0)
                                | (((fp_data>>23)&0xff) == 106) & ((fp_data&0xfffff) == 0)
                                | (((fp_data>>23)&0xff) == 105) & ((fp_data&0x1fffff) == 0)
                                | (((fp_data>>23)&0xff) == 104) & ((fp_data&0x3fffff) == 0)
                                | (((fp_data>>23)&0xff) == 103) & ((fp_data&0x7fffff) == 0);
    fp_inexact_subnormal_value = (((fp_data>>23)&0xff) < 113) & (((fp_data>>23)&0xff) > 102) & ~fp_exact_subnormal_value;
    fp_is_uf                   = ((((fp_data>>23)&0xff) < 103) & ~fp_is_zero) | fp_is_subnormal | fp_inexact_subnormal_value;

    if(((fp_data>>23)&0xff)==112)       fp_subnormal_fraction = (0x1<<9) | ((fp_data>>14)&0x1ff);
    else if(((fp_data>>23)&0xff)==111)  fp_subnormal_fraction = (0x1<<8) | ((fp_data>>15)&0xff);
    else if(((fp_data>>23)&0xff)==110)  fp_subnormal_fraction = (0x1<<7) | ((fp_data>>16)&0x7f);
    else if(((fp_data>>23)&0xff)==109)  fp_subnormal_fraction = (0x1<<6) | ((fp_data>>17)&0x3f);
    else if(((fp_data>>23)&0xff)==108)  fp_subnormal_fraction = (0x1<<5) | ((fp_data>>18)&0x1f);
    else if(((fp_data>>23)&0xff)==107)  fp_subnormal_fraction = (0x1<<4) | ((fp_data>>19)&0xf);
    else if(((fp_data>>23)&0xff)==106)  fp_subnormal_fraction = (0x1<<3) | ((fp_data>>20)&0x7);
    else if(((fp_data>>23)&0xff)==105)  fp_subnormal_fraction = (0x1<<2) | ((fp_data>>21)&0x3);
    else if(((fp_data>>23)&0xff)==104)  fp_subnormal_fraction = (0x1<<1) | ((fp_data>>22)&0x1);
    else if(((fp_data>>23)&0xff)==103)  fp_subnormal_fraction = (0x1<<0);
    else                                fp_subnormal_fraction = 0;

    fp_conv_exp_truncate       = ((fp_data>>23)&0xff) - 112;
    if(fp_is_nan) fp_conv_sign = 0;
    else          fp_conv_sign = (fp_data>>31)&0x1;

    if(fp_is_nan)             fp_conv_exp = 0x1f;
    else if(fp_is_infinite)   fp_conv_exp = 0x1f;
    else if(fp_is_uf)         fp_conv_exp = 0x0;
    else if(fp_is_zero)       fp_conv_exp = 0x0;
    else if(fp_is_of)         fp_conv_exp = 0x1e;
    else if(fp_exact_subnormal_value)       fp_conv_exp = 0x0;
    else                                    fp_conv_exp = fp_conv_exp_truncate&0x1f;

    if(fp_is_nan)                                                  fp_conv_fraction = 0x200;
    else if(fp_is_uf & ~fp_inexact_subnormal_value)                fp_conv_fraction = 0x0;
    else if(fp_is_of)                                              fp_conv_fraction = 0x3ff;
    else if(fp_exact_subnormal_value | fp_inexact_subnormal_value) fp_conv_fraction = fp_subnormal_fraction;
    else                                                           fp_conv_fraction = (fp_data>>13)&0x3ff;

    fp_out = ((fp_conv_sign&0x1)<<15) | ((fp_conv_exp&0x1f)<<10) | ((fp_conv_fraction&0x3ff)<<0);
    return fp_out;
}
