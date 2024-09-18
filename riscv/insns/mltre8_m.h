// MTU_TR_LD(0, 'c', uint8, true);
const reg_t baseAddr = RS1; 
  const reg_t stride2 = RS2; 
  const reg_t td = insn.td(); 
  reg_t lmul = 1; 
  require_align(td, lmul); 
  reg_t height, width; 
  reg_t rmax = 0, cmax = 0;
  MTU_LS_LEN(0, 'c', sizeof(uint8_t)); 
  WHOLE_MATRIX(0, true) 
  CLEAR_TILE(td); 
  for (reg_t i = 0; i < height; ++i) { 
    for (reg_t m = 0; m < lmul; m++) {
      for (reg_t j = 0; j < width; ++j) { 
          uint8_t val = MMU.load<uint8_t>( 
                    baseAddr + i * stride2 + j * sizeof(uint8_t) + 
                      m * (width * sizeof(uint8_t)));
          P.MU.tr_elt<uint8_t>(td + m, 0, i, j, rmax, cmax, false, true) = val; 
      } 
    } 
  } 