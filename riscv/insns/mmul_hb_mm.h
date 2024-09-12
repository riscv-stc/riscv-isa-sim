// mmul td ts1, ts2;
MI_MM_LOOP_E4({
    bit4_pair_t<int8_t> ts1_bit4 = ts1;
    bit4_pair_t<int8_t> ts2_bit4 = ts2;
    td = ts1_bit4 * ts2_bit4;
}, X2D, true, e4)