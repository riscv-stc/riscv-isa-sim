// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> (64 - P.MU.msew);
int64_t int_min = INT64_MIN >> (64 - P.MU.msew);

MI_MM_LOOP
({
  bool overflow = false;
  int128_t result  = 0;
  
  if (P.MU.msew != e4) {
    result = (int128_t)ts1 * (int128_t)ts2;
  } else {
    bit4_pair_t <int8_t> paire_bit4_ts1(ts1);
    bit4_pair_t <int8_t> paire_bit4_ts2(ts2);
    if (j % 2 == 0) {
      result = (int128_t)paire_bit4_ts1.high * (int128_t)paire_bit4_ts2.high;
    } else {
      result = (int128_t)paire_bit4_ts1.low * (int128_t)paire_bit4_ts2.low;
    }
  }
  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);
  // remove guard bits
//   result = result >> (sew - 1);

    if ( result > int_max){
        overflow = true;
        result = int_max;
    }else if (result < int_min){
        overflow = true;
        result = int_min;
    }

  // saturation
  if (overflow) {
    // result = int_max;
     P_SET_OV(1); 
  }

  if (P.MU.msew != e4) {
    td = result;
  } else {
    if (j % 2 == 0) {
      td = (result << 4);
    } else {
      td = (result & 0xF) | (td & 0xF0); 
    }
  }

//   td = result;
}, X2D, true, P.MU.msew)
