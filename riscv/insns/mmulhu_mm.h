// mmulhu td ts1, ts2;
MI_MM_LOOP({
    if (P.MU.msew != e4) {
        td = ((uint128_t)ts1 * ts2) >> sew;
    } else {
        bit4_pair_t<uint8_t> bit4_pair_ts1 = ts1;
        bit4_pair_t<uint8_t> bit4_pair_ts2 = ts2;
        uint32_t result = 0;
        if (j % 2 == 0) { 
            result = (uint32_t)bit4_pair_ts1.high * (uint32_t)bit4_pair_ts2.high;
            td = ((result >> 4) << 4) ; 
        } else { 
            result = (uint32_t)bit4_pair_ts1.low * (uint32_t)bit4_pair_ts2.low;
            td = ((result >> 4) & 0xF) | (td & 0xF0); 
        } 
    }
}, XU2D, true, P.MU.msew)