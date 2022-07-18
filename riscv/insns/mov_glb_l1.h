DMAE_DTYPE_DECODING_TO_ESIZE();

check_trap_mov_glb_l1(esize);

uint32_t data_type = DMAE_DATA_TYPE;
struct DmaeShapeStride dmae_ss;
dmae_ss.shape_x = DMAE_SHAPE_X;
dmae_ss.shape_y = DMAE_SHAPE_Y;
dmae_ss.shape_z = DMAE_SHAPE_Z;

dmae_ss.stride_s_x = DMAE_STRIDE_S_X;
dmae_ss.stride_s_y = DMAE_STRIDE_S_Y;
dmae_ss.stride_d_x = DMAE_STRIDE_D_X;
dmae_ss.stride_d_y = DMAE_STRIDE_D_Y;

dmae_ss.channel = DMAE_CHANNEL;

uint64_t len = dmae_dst_len(data_type, &dmae_ss);
// check_trap_mmu_pmp_ok(zext_xlen(RD), len, STORE, PRV_U);

// dmae_mov(src, dst, data_type, &dmae_ss);
dmae_vm_mov((reg_t)(RS1), (reg_t)(zext_xlen(RD)), data_type, &dmae_ss, p);

uint8_t* dst = (uint8_t*)p->get_sim()->addr_to_mem(zext_xlen(RD));
WRITE_MEM_STC(RD, (uint8_t*)dst, CMT_LOG_DMAE); 