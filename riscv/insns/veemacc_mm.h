require_extension('V');
NCP_AUNIT();

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd  = MMU.get_phy_addr(RD);
float16_t f16;

if(DIM == 0){
	CusIns.veemacc_mm((half*)rs1, (half*)&f16.v, (half*)rs2, &sst);
	WRITE_FRD(f16_to_f32(f16));
}
else{
	CusIns.veemacc_mm((half*)rs1, (half*)rd, (half*)rs2, &sst, DIM_DM);
}
