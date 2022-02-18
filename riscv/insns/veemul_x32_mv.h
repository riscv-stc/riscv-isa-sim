check_traps_vexxx_mv_x32;

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 4, 2);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rs2 = MMU.npc_addr_to_mem(RS2);
unsigned long rd = MMU.npc_addr_to_mem(RD);

CusIns.veemul_x32_mv((int32_t *)rs1, (half*)rd, (half*)rs2, &sst);
WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
