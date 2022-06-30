uint8_t e_size = 2;

if (MTE_DATA_TYPE == 0x0 || MTE_DATA_TYPE == 0x101)
    e_size = 2;
else if (MTE_DATA_TYPE == 0x202)
    e_size = 4;
else
    e_size = 1;

check_traps_icmov_m(e_size);

auto dst_coreId = p->idtu->idtu_coreid_trans(DST_CORE_ID);

//src shape
// #define ICMOV_OUTPUT_MSG
#ifdef ICMOV_OUTPUT_MSG
//output debug message
std::cout << " " << std::endl;
std::cout << "insn: icmov.m" << std::endl;
std::cout << "core id:" << std::dec << p->get_csr(CSR_TID) << std::endl;

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

std::cout << "target core: " << std::dec << dst_coreId << std::endl;

std::cout << "src addr: 0x" << std::hex << zext_xlen(RS1) << std::endl;
std::cout << "dst addr: 0x" << std::hex << reg_t(RD) << std::endl;
std::cout << " " << std::endl;
#endif

uint16_t col = MTE_SHAPE_COLUMN;
uint16_t row = MTE_SHAPE_ROW; 
uint32_t copy_stride_rs1 = (MTE_STRIDE_RS1 ? MTE_STRIDE_RS1 : col) * e_size;
uint32_t copy_stride_rd = (MTE_STRIDE_RD ? MTE_STRIDE_RD : col) * e_size;

auto src = (uint8_t*)MMU.mte_addr_to_mem(RS1);
auto dst = (uint8_t*)MMU.mte_addr_to_mem(zext_xlen(RD),dst_coreId);
assert(dst != nullptr && src != nullptr);

if ((MTE_STRIDE_RD == 0) && (MTE_STRIDE_RS1 == 0)) {
    memcpy(dst, src, col * row * e_size);
}
else {
    for (int i = 0; i < row; i++) {
    memcpy(dst + i * copy_stride_rd, src + i * copy_stride_rs1, col * e_size);
    }
    WRITE_MEM_STC(RD, (uint8_t*)dst, CMT_LOG_MTE);   
}
//icmov_vm(RS1, zext_xlen(RD), e_size, (uint32_t)DST_CORE_ID, (const struct MteShapeStride *)&mte_ss, p);
