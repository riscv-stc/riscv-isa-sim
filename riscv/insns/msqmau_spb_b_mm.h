VRM xrm = P.VU.get_vround_mode();
uint64_t int_max = UINT64_MAX >> (64 - e8 * 4);

MXU_SPB_QMM_LOOP
({
  if (P.MU.msew > e16)
    throw trap_illegal_instruction(insn.bits()); 
  bool overflow = false;
  uint128_t result = (uint128_t)ts1 * (uint128_t)ts2 + (uint128_t)accd;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // remove guard bits
  // result = result >> (sew - 1);
  if (result >= int_max)
    overflow = true;

  // max saturation
  if (overflow) {
    result = int_max;
    P_SET_OV(1);
  }

  accd = result;
}, USIGN, 4, e8)