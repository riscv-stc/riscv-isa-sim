VRM xrm = P.VU.get_vround_mode();
uint64_t int_max = UINT64_MAX >> (64 - P.MU.msew * 8);

MXU_SPB_OMM_LOOP
({
  if (P.MU.msew >= e16)
    throw trap_illegal_instruction(insn.bits()); 
  bool overflow = false;
  
  uint128_t result = 0;
  
  for (uint8_t i = 0 ; i < 2; i++){
        if (!i)
            result = (uint128_t)(ts1 & 0xF) * (uint128_t)(ts2 & 0xF) + (uint128_t)accd;
        else
            result = (uint128_t)(ts1 >> 4) * (uint128_t)(ts2 >> 4) + (uint128_t)accd;
        if (result >= int_max)
        overflow = true;

        // max saturation
        if (overflow) {
            result = int_max;
            P_SET_OV(1);
        }
        accd = result;
  }
  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // remove guard bits
  // result = result >> (sew - 1);

}, USIGN, 8, P.MU.msew)