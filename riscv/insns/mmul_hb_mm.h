// mmul td ts1, ts2;
MI_MM_LOOP({
    bit4_pair_t<int8_t> bit4_pair_ts1 = ts1;
    bit4_pair_t<int8_t> bit4_pair_ts2 = ts2;
    int8_t result = 0;
    if (j % 2 == 0) { 
        result = bit4_pair_ts1.high * bit4_pair_ts2.high;
        td = (result << 4) ; 
    } else { 
        result = bit4_pair_ts1.low * bit4_pair_ts2.low;
        td = (result & 0xF) | (td & 0xF0); 
    } 
}, X2D, true, e4)