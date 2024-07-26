MRM xrm = P.MU.get_mround_mode();
uint64_t int_max = UINT64_MAX >> (64 - P.MU.msew * 4);

MXU_MM_LOOP_QUEN
({
  if (P.MU.msew > e16)
    throw trap_illegal_instruction(insn.bits()); 
  bool overflow = false;
  uint128_t result = (uint128_t)ts1 * (uint128_t)ts2 + (uint128_t)td;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // remove guard bits
  // result = result >> (sew - 1);
  if (result >= int_max)
    overflow = true;

  // max saturation
  if (overflow) {
    result = int_max;
    P_SET_OM(1);
  }

  td = result;
}, MULXU, 4)