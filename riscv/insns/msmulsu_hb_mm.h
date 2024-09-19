// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> (64 - e4) ;
int64_t int_min = INT64_MIN >> (64 - e4);

MI_MM_LOOP_E4
({

  bool overflow = false;
  int4_bit_pair paire_bit4_ts1 = ts1;
  uint4_bit_pair paire_bit4_ts2 = ts2;

  int128_t result_low = (uint128_t)paire_bit4_ts1.low * (uint128_t)paire_bit4_ts2.low;
  int128_t result_high = (uint128_t)paire_bit4_ts1.high * (uint128_t)paire_bit4_ts2.high;


  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  if (result_low > int_max){
    result_low = int_max;
    overflow = true;
  } else if  (result_low < int_min) {
    overflow = true;
    result_low = int_min;
  }

  if (result_high > int_max){
    result_high = int_max;
    overflow = true;
  } else if  (result_high < int_min) {
    overflow = true;
    result_high = int_min;
  }

  // remove guard bits
//   result = result >> (sew - 1);

  // saturation
  if (overflow) {
    // result = int_max;
     P_SET_OV(1); 
  }

  td = (result_low & 0xF) | (result_high << 0x4);
}, XXU, true, e4)                                                                                