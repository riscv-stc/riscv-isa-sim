require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

if(DIM == 0){
	float32_t frs1 = f32(FRS1);
	half vr1;
	vr1.x = (unsigned short)frs1.v;
	CusIns.veacc_m(&vr1, (half*)rd, &sst);
}
else{
	CusIns.veacc_m((half*)rs1, (half*)rd, &sst, DIM_DM);
}
