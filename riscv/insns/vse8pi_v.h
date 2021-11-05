// vse8pi.v and vsseg[2-8]e8.v
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(uint8_t));
VI_ST(0, (i * nf + fn), uint8);
