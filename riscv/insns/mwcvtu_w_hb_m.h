MX_WQCVT(
{
    uint32_t &accd1  = P.MU.acc_elt<uint32_t>(td_num + m, \
        0, i, j * 2, mmax, nmax / 4 * amul, reg_rename, true); 
    uint4_bit_pair acc1_bit4_pair = acc1;
    accd1 = acc1_bit4_pair.low;
    uint32_t &accd2  = P.MU.acc_elt<uint32_t>(td_num + m, \
        0, i, j * 2 + 1, mmax, nmax / 4 * amul, reg_rename, true); 
    accd2 = acc1_bit4_pair.high;
    
}, SUDU_WIDE, 4, e4)