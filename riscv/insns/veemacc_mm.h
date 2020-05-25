require_extension('V');
check_ncp_vill_invalid()

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rs2 = MMU.get_phy_addr(RS2);
float16_t f16;

if(DIM == 0){
    check_traps_vexxx_mm_reduce_all;
	CusIns.veemacc_mm((half*)rs1, (half*)&f16.v, (half*)rs2, &sst);
	WRITE_FRD(f16_to_f32(f16));
}
else{
    check_traps_vexxx_mm_reduce_vector;
    unsigned long rd  = MMU.get_phy_addr(RD);
	CusIns.veemacc_mm((half*)rs1, (half*)rd, (half*)rs2, &sst, DIM_DM);
}
