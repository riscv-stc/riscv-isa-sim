// vle16pi.v and vlseg[2-8]e16.v
require(P.VU.vstart == 0);

VI_LD_PI(0, (i * nf + fn), int16, false);
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(int16_t));
