// require(P.MU.mstart == 0);

VRM xrm = P.VU.get_vround_mode();
uint64_t uint_max = UINT64_MAX >> (64 - P.MU.msew);

MI_MM_LOOP
({

  bool overflow = false;
  uint128_t result = 0;

  if (P.MU.msew != e4) {
    result = (uint128_t)ts1 * (uint128_t)ts2;
  } else {
    bit4_pair_t <uint8_t> paire_bit4_ts1(ts1);
    bit4_pair_t <uint8_t> paire_bit4_ts2(ts2);
    if (j % 2 == 0) {
      result = (uint128_t)paire_bit4_ts1.high * (uint128_t)paire_bit4_ts2.high;
    } else {
      result = (uint128_t)paire_bit4_ts1.low * (uint128_t)paire_bit4_ts2.low;
    }
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
}, XU2D, true, P.MU.msew)