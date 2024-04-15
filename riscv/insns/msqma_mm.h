MRM xrm = P.MU.get_mround_mode();
int64_t int_max = INT64_MAX >> (64 - P.MU.msew * 4);
int64_t int_min = INT64_MIN >> (64 - P.MU.msew * 4);
MI_MM_LOOP_QUEN
({
  auto &td  = P.MU.tr_elt<type_sew_t<32>::type>(td_num, 0, i, j, mmax, nmax, false); 
}, {
  auto &td  = P.MU.tr_elt<type_sew_t<64>::type>(td_num, 0, i, j, mmax, nmax, false); 
},{
  // if (P.MU.msew > e16)
  //   throw trap_illegal_instruction(insn.bits()); 

  // bool overflow = false;
  // int128_t result = (int128_t)ts1 * (int128_t)ts2 + (int128_t)td;

  // // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // // remove guard bits
  // result = result >> (sew - 1);

  // // max saturation
  // if (overflow) {
  //   result = int_max;
  //   P_SET_OM(1);
  // }

  // td = result;
}, XDS)