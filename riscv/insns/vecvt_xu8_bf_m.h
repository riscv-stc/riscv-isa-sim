check_traps_vexxx_m_convert(int8_t, int16_t);

struct ShapeStride sst;
sst_fill(sst, 1, 1);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

require(STATE.frm < 0x5);
vecvt_xu8_bf_m((Bfloat16*)rs1, (uint8_t*)rd, &sst, STATE.frm);
WRITE_MEM_STC(RD, (uint8_t*)rd, CMT_LOG_VME_DATA8);
