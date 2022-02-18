require_extension('V');

reg_t t_rs1 = RS1;
reg_t t_rs2 = RS2;
reg_t t_rd  = RD;
reg_t t_deq = MME_DEQUANT_COEFF;

//p->run_async([p, t_rs1, t_rs2, t_rd, pc]() {
  class CustomInsns CusIns;
  struct ShapeStride sst;


  unsigned long rs1 = MMU.npc_addr_to_mem(t_rs1);
  unsigned long rs2 = MMU.npc_addr_to_mem(t_rs2);
  unsigned long rd  = MMU.npc_addr_to_mem(t_rd);
  unsigned long dequant = MMU.npc_addr_to_mem(t_deq);
  

  switch (p->get_csr(CSR_MME_DATA_TYPE)){
  case 0x0: // f16*f16 = f16
    check_traps_memul_mm(int16_t, int16_t, int16_t); 
    memul_sst_fill(sst, 2, 2, 2); 
    CusIns.memul_mm((half*)rs1, (half*)rs2, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x2: // f16*f16 = f32
    check_traps_memul_mm(int16_t, int16_t, int32_t); 
    memul_sst_fill(sst, 2, 2, 4); 
    CusIns.memul_mm((half*)rs1, (half*)rs2, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x10101: // bf16*bf16 = bf16
    check_traps_memul_mm(int16_t, int16_t, int16_t); 
    memul_sst_fill(sst, 2, 2, 2); 
    CusIns.memul_mm((Bfloat16*)rs1, (Bfloat16*)rs2, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x10102: // bf16*bf16 = f32
    check_traps_memul_mm(int16_t, int16_t, int32_t); 
    memul_sst_fill(sst, 2, 2, 4); 
    CusIns.memul_mm((Bfloat16*)rs1, (Bfloat16*)rs2, (Float32*)rd, &sst);
    WRITE_MEM_STC(RD, (Float32*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x020202: // f32 * f32 = f32
    check_traps_memul_mm(int32_t, int32_t, int32_t); 
    memul_sst_fill(sst, 4, 4, 4);
    CusIns.memul_mm((float32_t*)rs1, (float32_t*)rs2, (float32_t*)rd, &sst);
    WRITE_MEM_STC(RD, (float32_t*)rd, CMT_LOG_MME_MEMUL_MM);  
    break;
    
  case 0x03030b: // int8 * int8 = f16(int32->f16,mf)
    check_traps_memul_mm(int8_t, int8_t, int16_t); 
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((int8_t*)rs1, (int8_t*)rs2, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x03030c: // int8 * int8 = f16(int32->f16,mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int8_t, int8_t, int16_t); 
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((int8_t*)rs1, (int8_t*)rs2, (half*)rd, &sst, (half *)dequant);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x03040b: // uint8 * int8 = f16(int32->f16,mf)
    check_traps_memul_mm(int8_t, int8_t, int16_t); 
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((uint8_t*)rs1, (int8_t*)rs2, (half*)rd, &sst);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x03040c: // uint8 * int8 = f16(int32->f16,mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int8_t, int8_t, int16_t); 
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((uint8_t*)rs1, (int8_t*)rs2, (half*)rd, &sst, (half *)dequant);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x03030f: // int8 * int8 = bf16(int32->bf16, mf)
    check_traps_memul_mm(int8_t, int8_t, int16_t);
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((int8_t*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030310: // int * int8 = bf16(int32->bf16, mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int8_t, int8_t, int16_t);
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((int8_t*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst, (Bfloat16 *)dequant);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x03040f: // uint8 * int8 = bf16(int32->bf16, mf)
    check_traps_memul_mm(int8_t, int8_t, int16_t);
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((uint8_t*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030410: // uint * int8 = bf16(int32->bf16, mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int8_t, int8_t, int16_t);
    memul_sst_fill(sst, 1, 1, 2);
    CusIns.memul_mm((uint8_t*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst, (Bfloat16 *)dequant);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;  

  case 0x03090b: // (float16->int8,mf) * int8 = f16(int32->f16, mf)
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((half*)rs1, (int8_t*)rs2, (half*)rd, &sst, true);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030a0b: // (float16->uint8,mf) * int8 = f16(int32->f16, mf)
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((half*)rs1, (int8_t*)rs2, (half*)rd, &sst, false);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x03090c: // (float16->int8) * int8 = f16 (int32->f16, mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((half*)rs1, (int8_t*)rs2, (half*)rd, &sst, true, (half *)dequant);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030a0c: // (float16->uint8) * int8 = f16 (int32->f16, mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((half*)rs1, (int8_t*)rs2, (half*)rd, &sst, false, (half *)dequant);
    WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030d0f: // (bfloat16->int8,mf) * int8 = bf16(int32->bf16, mf)
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((Bfloat16*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst, true);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030e0f: // (bfloat16->uint8,mf) * int8 = bf16(int32->bf16, mf)
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((Bfloat16*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst, false);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030d10: // (bfloat16->int8,mf) * int8 = bf16(int32->bf16, mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((Bfloat16*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst, true, (Bfloat16 *)dequant);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;
  case 0x030e10: // (bfloat16->uint8,mf) * int8 = bf16(int32->bf16, mv)
    check_traps_int8_mv(TRAP_MATRIX);
    check_traps_memul_mm(int16_t, int8_t, int16_t); 
    memul_sst_fill(sst, 2, 1, 2);
    CusIns.memul_mm((Bfloat16*)rs1, (int8_t*)rs2, (Bfloat16*)rd, &sst, false, (Bfloat16 *)dequant);
    WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_MME_MEMUL_MM);
    break;

  default:
    check_cust_invalid_mme_matrix_conv_data_type(p->get_csr(CSR_MME_DATA_TYPE));
    check_cust_invalid_npu_data_type(p->get_csr(CSR_MME_DATA_TYPE));
    break;  
  }
//});

//wfi();
