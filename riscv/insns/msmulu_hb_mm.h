// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
uint64_t uint_max = UINT64_MAX >> (64 - e4);

MI_MM_LOOP_E4
({

  bool overflow = false;
  uint4_bit_pair paire_bit4_ts1(ts1);
  uint4_bit_pair paire_bit4_ts2(ts2);
  uint4_bit_pair paire_bit4_temp;
  uint128_t result = (uint128_t)paire_bit4_ts1.low * (uint128_t)paire_bit4_ts2.low;
  uint128_t result1 = (uint128_t)paire_bit4_ts1.high * (uint128_t)paire_bit4_ts2.high;
  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);
  // remove guard bits
//   result = result >> (sew - 1);
  
  if (result >= uint_max){
    overflow = true;
    paire_bit4_temp.low = uint_max;
  } else {
    paire_bit4_temp.low = result;
  }

  if (result1 >= uint_max){
    overflow = true;
    paire_bit4_temp.high = uint_max;
  } else {
    paire_bit4_temp.high = result1;
  }

  // saturation
  if (overflow) {
    // result = int_max;
    P_SET_OV(1); 
  }
    td = paire_bit4_temp;
//   td = result;
}, XU2D, true, e4)