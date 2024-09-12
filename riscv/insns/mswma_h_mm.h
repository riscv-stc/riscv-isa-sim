VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> 32;
int64_t int_min = INT64_MIN >> 32;


MXU_MM_LOOP_WIDE
({
   if (P.MU.msew > e32)
    throw trap_illegal_instruction(insn.bits()); 

  bool overflow = false;
  int128_t result = (int128_t)ts1 * (int128_t)ts2 + (int128_t)accd;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // // remove guard bits
  // result = result >> (sew - 1);

  if (result > int_max){
    overflow = true;
    accd = int_max;
  }
  else if (result < int_min){
    overflow = true;
    accd = int_min;
  }
  else{
    accd = result;
  }

    
  // max saturation
  if (overflow) {
    P_SET_OV(1);
  }

  
}, MULX, 2, e16)