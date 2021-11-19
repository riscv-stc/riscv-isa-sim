// vle16pi.v and vlseg[2-8]e16.v
VI_LD_PI(0, (i * nf + fn), int16);
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(int16_t));
