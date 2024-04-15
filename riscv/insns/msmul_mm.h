require(P.MU.mstart == 0);

MRM xrm = P.MU.get_mround_mode();
int64_t int_max = INT64_MAX >> (64 - P.MU.msew);
int64_t int_min = INT64_MIN >> (64 - P.MU.msew);

MI_MM_LOOP
({
  bool overflow = false;

  int128_t result = (int128_t)ts1 * (int128_t)ts2;

  // rounding
  INT_ROUNDING(result, xrm, sew - 1);
  // remove guard bits
//   result = result >> (sew - 1);

    if ( result >= int_max){
        overflow = true;
        td = int_max;
    }else if (result <= int_max){
        overflow = true;
        td = int_min;
    }else{
        td = result;
    }

  // saturation
  if (overflow) {
    // result = int_max;
     P_SET_OM(1); 
  }

//   td = result;
}, X)
