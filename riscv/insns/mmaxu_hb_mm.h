MI_MM_LOOP_E4({
    uint8_t ts1_low = ts1 & 0xF;
    uint8_t ts2_low = ts2 & 0xF;
    uint8_t ts1_height = ts1 >> 0x4;
    uint8_t ts2_height = ts2 >> 0x4;
    uint8_t temp;
    if (ts1_low <= ts2_low)
        temp = ts2_low;
    else
        temp = ts1_low;
    
    if (ts1_height <= ts2_height)
        td = (uint8_t)(ts2_height << 0x4 | temp);
    else
        td = (uint8_t)(ts1_height << 0x4 | temp);
}, XU2D, true, e4)