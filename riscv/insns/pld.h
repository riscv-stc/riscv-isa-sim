uint8_t e_size = 2;

if (MTE_DATA_TYPE == 0 || MTE_DATA_TYPE == 0x101)
    e_size = 2;
else if (MTE_DATA_TYPE == 0x202)
    e_size = 4;
else
    e_size = 1;

WRITE_RS1(MMU.mte_atu_trans(RS1));
WRITE_RS2(p->idtu->idtu_coremap_trans((uint32_t)RS2));
check_traps_pld(e_size);
check_tcp_pld_invalid_coremap(p->get_id(), RS2);

reg_t addr = zext_xlen(RS1);
reg_t dst_addr = RD;
reg_t rs2 = RS2;

std::function<void()> pldFunc = [p, insn, pc, xlen, addr, dst_addr, e_size, rs2](){
  uint8_t* src = (uint8_t*)MMU.mte_addr_to_mem(addr);
  uint8_t* dst = (uint8_t*)MMU.mte_addr_to_mem(dst_addr);
  uint32_t core_map = (uint32_t)rs2;

// #define PLD_OUTPUT_MSG

#ifdef PLD_OUTPUT_MSG
 //output case message
  std::cout << " " << std::endl;
  //std::cout << "case num:" << MMU.load_int32(0xc07f3934) << std::endl;
  std::cout << "insn: pld" << std::endl;
  std::cout << "core id: " << std::dec << p->get_csr(CSR_TID) <<std::endl;

  if (MTE_DATA_TYPE == 0x0)
    std::cout << "data type: fp16" << std::endl;
  else if (MTE_DATA_TYPE == 0x101)
    std::cout << "data type: bfp16" << std::endl;
  else if (MTE_DATA_TYPE == 0x202)
    std::cout << "data type: fp32" << std::endl;
  else if (MTE_DATA_TYPE == 0x303)
    std::cout << "data type: int8" << std::endl;

  std::cout << "high: " << std::dec << MTE_SHAPE_ROW << std::endl;
  std::cout << "width: " << std::dec << MTE_SHAPE_COLUMN << std::endl;

  std::cout << "src stride: " << std::dec << MTE_STRIDE_RS1 << std::endl;
  std::cout << "dst stride: " << std::dec << MTE_STRIDE_RD << std::endl;
  std::cout << "coremap: 0x" << std::hex << rs2 << std::endl;

  std::cout << "src addr: 0x" << std::hex << addr  << std::endl;
  std::cout << "dst addr: 0x" << std::hex << dst_addr  << std::endl;
  std::cout << " " << std::endl;
#endif

  // do sync for pld
  p->pld(core_map);
  p->set_pld_finish();

  #if 0
  //2020-01-04: the bit of current core is no need to set but the data will send
  core_map |= 0x1 << p->get_csr(CSR_TID);
  #endif

  //src shape
  uint16_t col = MTE_SHAPE_COLUMN;
  uint16_t row = MTE_SHAPE_ROW; 
  uint32_t copy_stride_rs1 = 0;
  uint32_t copy_stride_rd = 0;
  uint32_t core_id = p->get_csr(CSR_TID);

  if (core_map != 0 && (core_map & (1 << core_id))) {
    copy_stride_rs1 = (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 : col) * e_size;
    copy_stride_rd = (MTE_STRIDE_RD ? MTE_STRIDE_RD : col) * e_size;

    if ((MTE_STRIDE_RD == 0) && (MTE_STRIDE_RS1 == 0)) {
      memcpy(dst, src, col * row * e_size);
    }
    else {
      for (int i = 0; i < row; i++) {
        memcpy(dst + i * copy_stride_rd, src + i * copy_stride_rs1, col * e_size);
      }
    }
    //mte_vm_mov(addr, dst_addr, e_size, (const struct MteShapeStride *)&mte_ss, p, false, true);
  }
};
if (!p->get_sim()->getMultiCoreThreadFlag())
  p->run_async(pldFunc, true);
else{
  p->set_run_async_state_start(true);
  pldFunc();
  p->set_run_async_state_finish(true);
}
wfi();
