check_traps_vexxx_m_convert(int16_t, int16_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

vecvt_hf_x16_m((int16_t*)rs1, (half*)rd, &sst);
WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_DATA16);