check_traps_vexxx_m_convert(int16_t, int16_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

vecvt_bf_x16_m((int16_t*)rs1, (Bfloat16*)rd, &sst);
WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_DATA16);