require_extension('V');
// FIXME: should implement ncp access trap

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst);
unsigned long rs1 = MMU.get_phy_addr(RS1);
float16_t f16;

if(DIM == 0){
	CusIns.veacc_m((half*)rs1, (half*)&f16.v, &sst);
	WRITE_FRD(f16_to_f32(f16));
}
else{
    unsigned long rd = MMU.get_phy_addr(RD);
    CusIns.veacc_m((half*)rs1, (half*)rd, &sst, DIM_DM);
}
