
MI_MM_LOOP_WIDEN
({
  MXU_WIDE_OP_AND_ASSIGN_MIX(acc1, acc2, 0, *, +, int, int, uint, P.MU.msew);
}, MULXXU, true, P.MU.msew)