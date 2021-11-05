// vle16pi.v and vlseg[2-8]e16.v
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(int16_t));
VI_LD(0, (i * nf + fn), int16);
