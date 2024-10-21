VRM xrm = P.VU.get_vround_mode();
int64_t int_max = INT64_MAX >> (64 - P.MU.msew);
int64_t int_min = INT64_MIN >> (64 - P.MU.msew);

MXU_SPA_MM_LOOP
({
  bool overflow = ts1 == ts2 && ts1 == int_min;
  int128_t result = (int128_t)ts1 * (int128_t)ts2 + (int128_t)accd;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  if (result > int_max){
    accd = int_max;
    overflow = true;
  }
  else if (result < int_min){
    accd = int_min;
    overflow = true;
  }
  else
    accd = result;
  // remove guard bits
  // result = result >> (sew - 1);

  // max saturation
  if (overflow) {
    // result = int_max;
    P_SET_OV(1);
    
  }

}, SIGN, 1, P.MU.msew)
