MI_MM_LOOP({
    bool sat = false;
    td = ts1 + ts2;
    sat = td < ts1;
    td |= -(td < ts1);
    P_SET_OM(sat);
},XU, true)