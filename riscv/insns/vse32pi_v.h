// vse32pi.v and vsseg[2-8]e32.v
VI_ST_PI(0, (i * nf + fn), uint32, false);
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(uint32_t));
