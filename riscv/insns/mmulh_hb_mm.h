MI_MM_LOOP({
    int32_t result = 0;
    bit4_pair_t<int8_t> bit4_pair_ts1 = ts1;
    bit4_pair_t<int8_t> bit4_pair_ts2 = ts2;
    if (j % 2 == 0) { 
        result = (int32_t)bit4_pair_ts1.high * (int32_t)bit4_pair_ts2.high;
        td = ((result >> 4) << 4) ; 
    } else { 
        result = (int32_t)bit4_pair_ts1.low * (int32_t)bit4_pair_ts2.low;
        td = ((result >> 4) & 0xF) | (td & 0xF0); 
    } 
}, XU2D, true, e4)