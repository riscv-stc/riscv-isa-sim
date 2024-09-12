MI_MM_LOOP({
    td = ((int128_t)ts1 * (uint128_t)ts2) >> sew;
}, XSU, true, e16)