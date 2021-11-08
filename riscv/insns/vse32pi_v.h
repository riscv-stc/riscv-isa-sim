// vse32pi.v and vsseg[2-8]e32.v
WRITE_RS1(RS1 + (P.VU.vl)*sizeof(uint32_t));
VI_ST(0, (i * nf + fn), uint32);
