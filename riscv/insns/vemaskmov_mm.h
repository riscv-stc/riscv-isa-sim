require_extension('V');

struct ShapeStride sst;

sst_fill(sst, 1, 1);

unsigned long rs2 = MMU.npc_addr_to_mem(RS2);
unsigned long rs1 = MMU.npc_addr_to_mem(RS1);
unsigned long rd = MMU.npc_addr_to_mem(RD);

// #define VEMASKMOV_OUTPUT_MSG 

#ifdef VEMASKMOV_OUTPUT_MSG
//output debug message
uint16_t col = SHAPE1_COLUMN;
uint16_t row = SHAPE1_ROW; 

std::cout << " " << std::endl;
std::cout << "insn: vemaskmov_mm" << std::endl;

if (VME_DATA_TYPE == 0x0)
  std::cout << "data type: fp16 " << std::endl;
else if (VME_DATA_TYPE == 0x10101)
  std::cout << "data type: bf16" << std::endl;
else if (VME_DATA_TYPE == 0x20202)
  std::cout << "data type: fp32" << std::endl;

std::cout << "high: " << std::dec << row << std::endl;
std::cout << "width: " << std::dec << col << std::endl;

std::cout << "src stride: " << std::dec << STRIDE_RS1 << std::endl;
std::cout << "dst stride: " << std::dec << STRIDE_RD << std::endl;

std::cout << "src rs1 addr: 0x" << std::hex << zext_xlen(RS1) << std::endl;
std::cout << "src rs2 addr: 0x" << std::hex << zext_xlen(RS2) << std::endl;
std::cout << "dst addr: 0x" << std::hex << reg_t(RD) << std::endl;
std::cout << " " << std::endl;
#endif

VME_DTYPE_DECODING_TO_TYPE({
    check_traps_vexxx_mm(dtype_lut);
    vemaskmov_mm<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, (dtype_vd*)rs2, &sst);
    WRITE_MEM_STC(RD, (dtype_vd*)rd, CMT_LOG_VME_COM); 
})