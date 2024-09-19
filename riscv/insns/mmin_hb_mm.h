MI_MM_LOOP_E4({
    int8_t ts1_low = ts1 & 0xF;
    int8_t ts2_low = ts2 & 0xF;
    int8_t ts1_height = ts1 >> 0x4;
    int8_t ts2_height = ts2 >> 0x4;
    int8_t temp;
    BIT4_SIGN_EXTEND_INT8(int8_t, ts1_low);
    BIT4_SIGN_EXTEND_INT8(int8_t, ts2_low);
    BIT4_SIGN_EXTEND_INT8(int8_t, ts1_height);
    BIT4_SIGN_EXTEND_INT8(int8_t, ts2_height);
    if (ts1_low <= ts2_low)
        temp = ts1_low;
    else
        temp = ts2_low;
    
    if (ts1_height <= ts2_height)
        td = (int8_t)(ts1_height << 0x4 | temp);
    else
        td = (int8_t)(ts2_height << 0x4 | temp);
}, X2D, true, e4)