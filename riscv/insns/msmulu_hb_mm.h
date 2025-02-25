// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
uint64_t uint_max = UINT64_MAX >> (64 - e4);

MI_MM_LOOP
({

  bool overflow = false;
  uint4_bit_pair paire_bit4_ts1(ts1);
  uint4_bit_pair paire_bit4_ts2(ts2);

  uint128_t result = 0;
  if (j % 2 == 0) {
    result = (uint128_t)paire_bit4_ts1.high * (uint128_t)paire_bit4_ts2.high;
  } else {
    result = (uint128_t)paire_bit4_ts1.low * (uint128_t)paire_bit4_ts2.low;
  }
  
  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);
  // remove guard bits
//   result = result >> (sew - 1);
  
  if (result >= uint_max){
    overflow = true;
    result = uint_max;
  } 

  // saturation
  if (overflow) {
    // result = int_max;
    P_SET_OV(1); 
  }
  if (j % 2 == 0) {
    td = (result << 4);
  } else {
    td = (result & 0xF) | (td & 0xF0); 
  }
}, XU2D, true, e4)