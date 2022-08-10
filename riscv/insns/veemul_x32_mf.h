check_traps_vexxx_mf_x32;

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, 4, 2);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);
half vr2;
vr2.x = f32_to_f16(f32(FRS2)).v;

CusIns.veemul_x32_mf((int32_t*)rs1, vr2, (half*)rd, &sst);
WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
