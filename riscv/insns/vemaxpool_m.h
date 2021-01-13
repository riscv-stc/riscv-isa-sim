require_extension('V');

struct VmeShapeStride vss;
vme_ss_fill(vss, 2);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

VME_DTYPE_DECODING_TO_TYPE({
    vemaxpool_m<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &vss, relu);
})
