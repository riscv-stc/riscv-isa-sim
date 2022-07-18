check_traps_vexxx_m_convert(int32_t, int32_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

require(STATE.frm < 0x5);
vecvt_x32_f32_m((Float32*)rs1, (int32_t*)rd, &sst, STATE.frm);
WRITE_MEM_STC(RD, (int32_t*)rd, CMT_LOG_VME_DATA32);