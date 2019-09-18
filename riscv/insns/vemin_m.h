require_extension('V');
NCP_AUNIT();

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
//unsigned long rs2 = MMU.get_phy_addr(RS2);
float16_t f16;

if(DIM == 0){
	CusIns.vemin_m((half*)rs1, (half*)&f16.v, &sst);
	WRITE_FRD(f16_to_f32(f16));
}
else{
    unsigned long rd = MMU.get_phy_addr(RD);
    CusIns.vemin_m((half*)rs1, (half*)rd, &sst, DIM_DM);
}
