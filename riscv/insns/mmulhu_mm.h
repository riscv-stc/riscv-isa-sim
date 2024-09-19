// mmulhu td ts1, ts2;
MI_MM_LOOP({
    td = ((uint128_t)ts1 * ts2) >> sew;
}, XU2D, true, P.MU.msew)