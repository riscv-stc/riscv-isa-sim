/*
uint8_t e_size = 2;

if (MTE_DATA_TYPE == 0x0 || MTE_DATA_TYPE == 0x101)
    e_size = 2;
else if (MTE_DATA_TYPE == 0x202)
    e_size = 4;
else
    e_size = 1;
*/
MTE_DTYPE_DECODING_TO_TYPE({
  uint8_t esize = 2;

  esize = sizeof(dtype_lut);
  check_traps_mov_l1_llb(dtype_lut);

  uint8_t* src = (uint8_t*)p->get_sim()->addr_to_mem(zext_xlen(RS1));
  uint8_t* dst = (uint8_t*)MMU.npc_addr_to_mem(RD);

  //src shape
  uint16_t col = MTE_SHAPE_COLUMN;
  uint16_t row = MTE_SHAPE_ROW;
  uint32_t copy_stride_rs1 = (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 : col) * esize;
  uint32_t copy_stride_rd = (MTE_STRIDE_RD ? MTE_STRIDE_RD : col) * esize;

// #define MOV_L1_LLB_OUTPUT_MSG
#ifdef MOV_L1_LLB_OUTPUT_MSG
if (MTE_DATA_TYPE == 0x0)
  std::cout << "data type: fp16" << std::endl;
else if (MTE_DATA_TYPE == 0x101)
  std::cout << "data type: bfp16" << std::endl;
else if (MTE_DATA_TYPE == 0x202)
  std::cout << "data type: fp32" << std::endl;
else if (MTE_DATA_TYPE == 0x303)
  std::cout << "data type: int8" << std::endl;

std::cout << "high: " << std::dec << row << std::endl;
std::cout << "width: " << std::dec << col << std::endl;

std::cout << "src stride: " << std::dec << MTE_STRIDE_RS1 << std::endl;
std::cout << "dst stride: " << std::dec << MTE_STRIDE_RD << std::endl;

std::cout << "src addr: 0x" << std::hex << zext_xlen(RS1)  << std::endl;
std::cout << "dst addr: 0x" << std::hex << (reg_t)RD  << std::endl;
std::cout << " " << std::endl;

#endif
  uint64_t len = row * copy_stride_rs1;
  check_trap_mmu_pmp_ok(zext_xlen(RS1), len, LOAD, PRV_U);

  if ((MTE_STRIDE_RD == 0) && (MTE_STRIDE_RS1 == 0)) {
      memcpy(dst, src, col * row * esize);
  }
  else {
    for (int i = 0; i < row; i++) {
      memcpy(dst + i * copy_stride_rd, src + i * copy_stride_rs1, col * esize);
    }
  }
  //mte_vm_mov((reg_t)(zext_xlen(RS1)), (reg_t)(RD), esize, (const struct MteShapeStride *)&mte_ss, p, false, true);

  WRITE_MEM_STC(RD, (uint8_t*)dst, CMT_LOG_MTE);  

})

