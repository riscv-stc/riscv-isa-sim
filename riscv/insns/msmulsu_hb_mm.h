// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> (64 - e4) ;
int64_t int_min = INT64_MIN >> (64 - e4);

MI_MM_LOOP
({

  bool overflow = false;
  int4_bit_pair paire_bit4_ts1 = ts1;
  uint4_bit_pair paire_bit4_ts2 = ts2;

  int128_t result = 0;

  if (j % 2 == 0) {
    result = (int128_t)paire_bit4_ts1.high * (uint128_t)paire_bit4_ts2.high;
  } else {
    result = (int128_t)paire_bit4_ts1.low * (uint128_t)paire_bit4_ts2.low;
  }

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  if (result > int_max){
    result = int_max;
    overflow = true;
  } else if  (result < int_min) {
    overflow = true;
    result = int_min;
  }

  // remove guard bits
//   result = result >> (sew - 1);

  // saturation
  if (overflow) {
    // result = int_max;
     P_SET_OV(1); 
  }

    if (j % 2 == 0) { 
      td = (result << 4) | (td & 0); 
    } else { 
      td = (result & 0xF) | (td & 0xF0); 
    } 

}, XXU, true, e4)                                                                                