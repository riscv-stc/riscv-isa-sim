MX_CVT(
{
    if (P.MU.msew != e4) {
        accd = acc1;
    } else {
        bit4_pair_t<uint8_t> acc1_bit4_pair = acc1;
        if (j % 2 == 0) {
            accd = acc1_bit4_pair.high << 4;
        } else {
            accd = (acc1_bit4_pair.low & 0xF) | (accd & 0xF0);
        }
    }
}, SUDS, 1, P.MU.msew)