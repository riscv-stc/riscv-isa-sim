VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> (64 - e4 * 8);
int64_t int_min = INT64_MIN >> (64 - e4 * 8);
MXU_SPA_OMM_LOOP
({
  if (P.MU.msew >= e16)
    throw trap_illegal_instruction(insn.bits()); 

  bool overflow = false;
  int128_t result = (int128_t)ts1 * (int128_t)ts2 + (int128_t)accd;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // remove guard bits
  // result = result >> (sew - 1);

  int4_bit_pair ts2_bit4(ts2); 
  int4_bit_pair ts1_bit4(ts1); 
    if (temp % 2 == 0 && j % 2 == 0) {
      result = ((int128_t)ts2_bit4.high * (int128_t)ts1_bit4.high) + (int128_t)accd; 

    }else if (temp % 2 == 0 && j % 2 != 0) {
      result = ((int128_t)ts2_bit4.low * (int128_t)ts1_bit4.high) + (int128_t)accd;
    }else if (temp % 2 != 0 && j % 2 == 0) {
      result = ((int128_t)ts2_bit4.high * (int128_t)ts1_bit4.low) + (int128_t)accd;
    }else {
      result = ((int128_t)ts2_bit4.low * (int128_t)ts1_bit4.low) + (int128_t)accd;
    }
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
            P_SET_OV(1);
        }

        accd = result;


}, SIGN, 8, e4)