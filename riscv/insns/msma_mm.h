MRM xrm = P.MU.get_mround_mode();
int64_t int_max = INT64_MAX >> (64 - P.MU.msew);
int64_t int_min = INT64_MIN >> (64 - P.MU.msew);
MI_MM_LOOP
({
  bool overflow = ts1 == ts2 && ts1 == int_min;
  int128_t result = (int128_t)ts1 * (int128_t)ts2 + (int128_t)td;

  // rounding
  INT_ROUNDING(result, xrm, sew - 1);

  if (result >= int_max){
    td = int_max;
    overflow = true;
  }
  else if (result <= int_min){
    td = int_min;
    overflow = true;
  }
  else
    td = result;
  // remove guard bits
  result = result >> (sew - 1);

  // max saturation
  if (overflow) {
    // result = int_max;
    P_SET_OM(1);
    
  }
    
//   td = result;
}, X)
