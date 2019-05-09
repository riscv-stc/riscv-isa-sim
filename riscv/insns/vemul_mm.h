require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
//sst.shape1_column = SHAPE1_COLUMN;
//sst.shape1_row = SHAPE1_ROW;
//sst.shape2_column = SHAPE2_COLUMN;
//sst.shape2_row = SHAPE2_ROW;
//sst.stride_rd = STRIDE_RD;
//sst.stride_rs1 = STRIDE_RS1;
//sst.stride_rs2 = STRIDE_RS2;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

CusIns.vemul_mm((half*)rs1, (half*)rs2, (half*)rd, &sst);
