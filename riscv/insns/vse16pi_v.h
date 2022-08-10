// vse16pi.v and vsseg[2-8]e16.v
require(P.VU.vstart == 0);

VI_ST_PI(0, (i * nf + fn), uint16, false);
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(uint16_t));
