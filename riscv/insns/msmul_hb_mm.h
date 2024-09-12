// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> (64 - e4);
int64_t int_min = INT64_MIN >> (64 - e4);

MI_MM_LOOP_E4
({
  bool overflow = false;
  int4_bit_pair ts1_bit4_pair(ts1);
  int4_bit_pair ts2_bit4_pair(ts2);
  int128_t result_low = (int128_t)ts1_bit4_pair.low * (int128_t)ts2_bit4_pair.low;
  int128_t result_high = (int128_t)ts1_bit4_pair.high * (int128_t)ts2_bit4_pair.high;
  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);
  // remove guard bits
//   result = result >> (sew - 1);

    if ( result_low > int_max){
        overflow = true;
        result_low = int_max;
    }else if (result_low < int_min){
        overflow = true;
        result_low = int_min;
    }

    if ( result_high > int_max){
        overflow = true;
        result_high = int_max;
    }else if (result_high < int_min){
        overflow = true;
        result_high = int_min;
    }

  // saturation
  if (overflow) {
    // result = int_max;
     P_SET_OV(1); 
  }
    td = (result_low & 0xF) | (result_high << 0x4);
//   td = result;
}, X2D, true, e4)
