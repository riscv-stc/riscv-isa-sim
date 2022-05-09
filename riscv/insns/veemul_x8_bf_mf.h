check_traps_vexxx_mf_x8out;

struct ShapeStride sst;

sst_fill(sst, 1, 1);
sst.stride_rd = STRIDE_RD;

unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

Bfloat16 vr2(f32(FRS2));
veemul_x8_bf_mf<signed char, Bfloat16>((Bfloat16 *)rs1, vr2, (signed char *)rd, &sst, STATE.frm);
WRITE_MEM_STC(RD, (signed char*)rd, CMT_LOG_VME_DATA8); 
