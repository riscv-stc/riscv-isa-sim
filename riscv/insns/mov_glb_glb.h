
uint32_t data_type = DMAE_DATA_TYPE;
struct DmaeShapeStride dmae_ss;
dmae_ss.shape_x = DMAE_SHAPE_X;
dmae_ss.shape_y = DMAE_SHAPE_Y;
dmae_ss.shape_z = DMAE_SHAPE_Z;

dmae_ss.stride_s_x = DMAE_STRIDE_S_X;
dmae_ss.stride_s_y = DMAE_STRIDE_S_Y;
dmae_ss.stride_d_x = DMAE_STRIDE_D_X;
dmae_ss.stride_d_y = DMAE_STRIDE_D_Y;

uint64_t len = 0;
len = dmae_src_len(data_type, &dmae_ss);
//check_trap_mmu_pmp_ok(zext_xlen(RS1), len, LOAD, PRV_U);

len = dmae_dst_len(data_type, &dmae_ss);
//check_trap_mmu_pmp_ok(zext_xlen(RD), len, STORE, PRV_U);

dmae_vm_mov((reg_t)(zext_xlen(RS1)), (reg_t)(zext_xlen(RD)), data_type, &dmae_ss, p, false, false);

WRITE_MEM_STC(RD, (uint8_t*)dst, CMT_LOG_DMAE);
