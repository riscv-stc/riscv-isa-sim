check_traps_vexxx_mf_x8out;

struct ShapeStride sst;

sst_fill(sst, 1, 1);
sst.stride_rd = STRIDE_RD;

unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

Bfloat16 vr2(f32(FRS2));
veemul_xu8_bf_mf<unsigned char, Bfloat16>((Bfloat16 *)rs1, (unsigned char *)rd, vr2, &sst, STATE.frm);
WRITE_MEM_STC(RD, (unsigned char*)rd, CMT_LOG_VME_DATA8);
