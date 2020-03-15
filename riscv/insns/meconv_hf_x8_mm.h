require_extension('V');
NCP_AUNIT();

class CustomInsns CusIns;
struct ConvShapeStride sst;
conv_fill(sst);

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

sst.dequant_coeff.x = f32_to_f16(f32(CONV_DEQUANT_COEFF)).v;;

CusIns.meconv_hf_x8_mm((int8_t*)rs1, (half *)rd, (int8_t*)rs2, &sst);

