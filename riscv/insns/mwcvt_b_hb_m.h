MX_WCVT(
{
    int4_bit_pair acc1_bit4_pair = acc1;
    if (j % 2 == 0) {
        accd = acc1_bit4_pair.high;
    } else {
        accd = acc1_bit4_pair.low ;
    }
    
}, SSDS_WIDE, 2, e4)