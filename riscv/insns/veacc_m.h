require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 2, 2);
unsigned long rs1 = MMU.get_phy_addr(RS1);
float16_t f16;

if(DIM == 0){
    check_traps_vexxx_m_reduce_all;
	CusIns.veacc_m((half*)rs1, (half*)&f16.v, &sst);
	WRITE_FRD(f16_to_f32(f16));
}
else{
    check_traps_vexxx_m_reduce_vector;
    unsigned long rd = MMU.get_phy_addr(RD);
    CusIns.veacc_m((half*)rs1, (half*)rd, &sst, DIM_DM);
}
