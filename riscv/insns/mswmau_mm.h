MRM xrm = P.MU.get_mround_mode();
uint64_t int_max = UINT64_MAX >> (64 - P.MU.msew);
bool overflow = false;

MI_MM_LOOP_WIDEN
({
  auto &td  = P.MU.tr_elt<type_usew_t<16>::type>(td_num, 0, i, j, mmax, nmax, false); 
}, {
  auto &td  = P.MU.tr_elt<type_usew_t<32>::type>(td_num, 0, i, j, mmax, nmax, false); 
}, {
  auto &td  = P.MU.tr_elt<type_usew_t<64>::type>(td_num, 0, i, j, mmax, nmax, false); 
}, {
  //  if (P.MU.msew > e32)
  //   throw trap_illegal_instruction(insn.bits()); 

  
  // uint128_t result = (uint128_t)ts1 * (uint128_t)ts2 + (uint128_t)td;

  // // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  // // // remove guard bits
  // // result = result >> (sew - 1);

  // if (result >= int_max)
  //   overflow = true;
    
  // // max saturation
  // if (overflow) {
  //   result = int_max;
  //   P_SET_OM(1);
  // }

  // td = result;
}, XDSU)