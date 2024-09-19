MI_MM_LOOP_E4({
    bit4_pair_t<int8_t> ts1_bit4_pair = ts1;
    bit4_pair_t<uint8_t> ts2_bit4_pair = ts2;
    td = ts1_bit4_pair.arithmetic_right_shift(ts2_bit4_pair);
}, XSU, true, e4)