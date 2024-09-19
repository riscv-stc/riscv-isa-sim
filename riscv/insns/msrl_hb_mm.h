MI_MM_LOOP_E4({
    bit4_pair_t<uint8_t> ts1_bit4_pair = ts1;
    bit4_pair_t<uint8_t> ts2_bit4_pair = ts2;
    td = ts1_bit4_pair >> ts2_bit4_pair;
}, XU2D, true, e4)