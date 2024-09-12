// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> (64 - e64) ;
int64_t int_min = INT64_MIN >> (64 - e64);

MI_MM_LOOP
({

  bool overflow = false;

  int128_t result = (int128_t)ts1 * (uint128_t)ts2;

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

  td = result;
}, XXU, true, e64) 