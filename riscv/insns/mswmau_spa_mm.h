VRM xrm = P.VU.get_vround_mode();
uint64_t uint_max = UINT64_MAX >> (64 - P.MU.msew * 2);


MXU_SPA_WMM_LOOP
({

   if (P.MU.msew > e32)
    throw trap_illegal_instruction(insn.bits()); 

  bool overflow = false;
  uint128_t result = (uint128_t)ts1 * (uint128_t)ts2 + (uint128_t)accd;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // // remove guard bits
  // result = result >> (sew - 1);

  if (result >= uint_max)
    overflow = true;
    
  // max saturation
  if (overflow) {
    result = uint_max;
    P_SET_OV(1);
  }

  accd = result;
}, USIGN, 2, P.MU.msew)
