MI_MM_LOOP({
    bit4_pair_t<uint8_t> bit4_pair_ts1 = ts1;
    bit4_pair_t<uint8_t> bit4_pair_ts2 = ts2;

    if (j % 2 == 0) { 
        td = ((bit4_pair_ts1.high << bit4_pair_ts2.high) << 4) | (td & 0); 
    } else { 
        td = ((bit4_pair_ts1.low << bit4_pair_ts2.low) & 0xF) | (td & 0xF0); 
    } 
}, XU2D, true, e4)