MI_MM_LOOP({
    if (P.MU.msew != e4) {
        if (ts1 <= ts2)
            td = ts1;
        else
            td = ts2;
    } else {
        bit4_pair_t<uint8_t> bit4_pair_ts1; 
        bit4_pair_t<uint8_t> bit4_pair_ts2; 
            
        bit4_pair_ts1 = ts1; 
        bit4_pair_ts2 = ts2; 
            
        if (j % 2 == 0) { 
            if (bit4_pair_ts1.high <= bit4_pair_ts2.high){
                td = (bit4_pair_ts1.high << 4) | (td & 0); 
            } else {
                td = (bit4_pair_ts2.high << 4) | (td & 0); 
            }
        } else { 
            if (bit4_pair_ts1.low <= bit4_pair_ts2.low) {
                td = (bit4_pair_ts1.low & 0xF) | (td & 0xF0); 
            } else {
                td = (bit4_pair_ts2.low & 0xF) | (td & 0xF0); 
            }
        } 
    }
}, XU2D, true, P.MU.msew)