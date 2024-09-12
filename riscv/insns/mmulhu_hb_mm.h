MI_MM_LOOP_E4({
    uint8_t ts1_low = ts1 & 0xF;
    uint8_t ts2_low = ts2 & 0xF;
    uint8_t ts1_height = ts1 >> 0x4;
    uint8_t ts2_height = ts2 >> 0x4;
    uint8_t temp;
    temp = (ts1_low * ts2_low) >> sew;
    td = ((((uint128_t)(ts1_height * ts2_height)) >> sew) << 0x4) | (temp & 0xF);
}, XU2D, true, e4)