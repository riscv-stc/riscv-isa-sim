check_traps_vexxx_m_convert(int32_t, int32_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

vecvt_f32_x32_m((int32_t*)rs1, (Float32*)rd, &sst);
WRITE_MEM_STC(RD, (Float32*)rd, CMT_LOG_VME_DATA32);
