require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
unsigned long rd = MMU.get_phy_addr(RD);

if(DIM == 0){
	CusIns.veacc_m((half*)rs1, (half*)rd, &sst);
}
else{
	CusIns.veacc_m((half*)rs1, (half*)rd, &sst, DIM_DM);
}
