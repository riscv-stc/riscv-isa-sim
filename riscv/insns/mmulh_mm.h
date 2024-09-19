// mmulh td ts1, ts2;
MI_MM_LOOP({
    td = ((int128_t)ts1 * ts2) >> sew;
}, X2D, true, P.MU.msew)