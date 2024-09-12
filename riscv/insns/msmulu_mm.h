// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
uint64_t uint_max = UINT64_MAX >> (64 - P.MU.msew);

MI_MM_LOOP
({

  bool overflow = false;

  uint128_t result = (uint128_t)ts1 * (uint128_t)ts2;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);
  // remove guard bits
//   result = result >> (sew - 1);
  
  if (result >= uint_max){
    overflow = true;
    td = uint_max;
  } else {
    td = result;
  }

  // saturation
  if (overflow) {
    // result = int_max;
    P_SET_OV(1); 
  }

//   td = result;
}, XU2D, true, P.MU.msew)