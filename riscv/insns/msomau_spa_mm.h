VRM xrm = P.VU.get_vround_mode();
uint64_t uint_max = UINT64_MAX >> (64 - P.MU.msew * 8);

MXU_SPA_OMM_LOOP
({
  if (P.MU.msew >= e16)
    throw trap_illegal_instruction(insn.bits()); 
  bool overflow = false;
  
  uint128_t result = 0;
  if (P.MU.msew != e4) {
    result = (uint128_t)ts1 * (uint128_t)ts2 + (uint128_t)accd;
  } else {
    uint4_bit_pair ts2_bit4(ts2); 
    uint4_bit_pair ts1_bit4(ts1); 
    if (temp % 2 == 0 && j % 2 == 0) {
      result = ((uint128_t)ts2_bit4.high * (uint128_t)ts1_bit4.high) + (uint128_t)accd; 

    }else if (temp % 2 == 0 && j % 2 != 0) {
      result = ((uint128_t)ts2_bit4.low * (uint128_t)ts1_bit4.high) + (uint128_t)accd;
    }else if (temp % 2 != 0 && j % 2 == 0) {
      result = ((uint128_t)ts2_bit4.high * (uint128_t)ts1_bit4.low) + (uint128_t)accd;
    }else {
      result = ((uint128_t)ts2_bit4.low * (uint128_t)ts1_bit4.low) + (uint128_t)accd;
    }
  }
    
  if (result >= uint_max)
    overflow = true;

    // max saturation
    if (overflow) {
        result = uint_max;
        P_SET_OV(1);
    }
    accd = result;
  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // remove guard bits
  // result = result >> (sew - 1);

}, USIGN, 8, P.MU.msew)