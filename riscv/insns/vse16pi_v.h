// vse16pi.v and vsseg[2-8]e16.v
VI_ST_PI(0, (i * nf + fn), uint16);
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(uint16_t));
