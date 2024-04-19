// mmul td ts1, ts2;
MI_MM_LOOP({
    td = ts1 * ts2;
}, X, true)