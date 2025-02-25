MI_MM_LOOP({
    bit4_pair_t<uint8_t> ts1_bit4_pair = ts1;
    bit4_pair_t<uint8_t> ts2_bit4_pair = ts2;
    if (j %2 == 0) {
        td = (ts1_bit4_pair.high >> ts2_bit4_pair.high) << 4;
    } else {
        td = ((ts1_bit4_pair.low >> ts2_bit4_pair.low) & 0xF) | (td & 0xF0);
    }
}, XU2D, true, e4)