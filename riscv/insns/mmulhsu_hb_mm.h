// mmulhsu td ts1, ts2;

MI_MM_LOOP_E4({
    int4_bit_pair ts1_bit4_pair = ts1;
    uint4_bit_pair ts2_bit4_pair = ts2;
    int128_t temp_low = (ts1_bit4_pair.low * ts2_bit4_pair.low) >> sew;
    int128_t temp_high = (ts1_bit4_pair.high * ts2_bit4_pair.high) >> sew;
    
    td = (temp_low & 0xF) | (temp_high << 0x4);
}, XSU, true, e4)