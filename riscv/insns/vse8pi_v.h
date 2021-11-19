// vse8pi.v and vsseg[2-8]e8.v
VI_ST_PI(0, (i * nf + fn), uint8);
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(uint8_t));
