// MTU_TR_LD(1, 'a', uint64, false);
const reg_t baseAddr = RS1; 
  const reg_t stride2 = RS2; 
  const reg_t td = insn.td(); 
  reg_t lmul = 1;
  require_align(td, lmul); 
  reg_t height, width; 
  reg_t rmax = 0, cmax = 0;
  MTU_LS_LEN(1, 'a', sizeof(uint64_t)); 
  WHOLE_MATRIX(1, false) 
  CLEAR_TILE(td); 
  for (reg_t i = 0; i < height; ++i) { 
    for (reg_t m = 0; m < lmul; m++) {
      for (reg_t j = 0; j < width; ++j) { 
          uint64_t val = MMU.load<uint64_t>( 
                    baseAddr + i * stride2 + j * sizeof(uint64_t) + 
                      m * (width * sizeof(uint64_t)));
          P.MU.tr_elt<uint64_t>(td + m, 1, i, j, rmax, cmax, false, true) = val; 
          printf("tr%d,val= 0x%llx\n",td + m, val);
      } 
    } 
  } 