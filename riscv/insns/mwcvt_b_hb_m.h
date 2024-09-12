MX_WCVT(
{
    int8_t &accd1  = P.MU.acc_elt<int8_t>(td_num + m, \
        0, i, j * 2, mmax, nmax * amul, reg_rename, true); 
    uint4_bit_pair acc1_bit4_pair = acc1;
    accd1 = acc1_bit4_pair.low;
    int8_t &accd2  = P.MU.acc_elt<int8_t>(td_num + m, \
        0, i, j * 2 + 1, mmax, nmax * amul, reg_rename, true); 
    accd2 = acc1_bit4_pair.high;
    
}, SSDS_WIDE, 1, e4)