MRM xrm = P.MU.get_mround_mode();
int64_t int_max = INT64_MAX >> (64 - P.MU.msew * 4);
int64_t int_min = INT64_MIN >> (64 - P.MU.msew * 4);
MXU_MM_LOOP_QUEN
({
  if (P.MU.msew > e16)
    throw trap_illegal_instruction(insn.bits()); 

  bool overflow = false;
  int128_t result = (int128_t)ts1 * (int128_t)ts2 + (int128_t)td;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // remove guard bits
  // result = result >> (sew - 1);

  if (result > int_max){
    overflow = true;
    result = int_max;
  } else if (result < int_min){
    overflow = true;
    result = int_min;
  }
  // max saturation
  if (overflow) {
    // result = int_max;
    P_SET_OM(1);
  }

  td = result;
}, MULX)