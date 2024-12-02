VRM xrm = P.VU.get_vround_mode();
uint64_t uint_max = UINT64_MAX;
MXU_SPB_MM_LOOP
({
  bool overflow = false;
  uint128_t result = (uint128_t)ts1 * (uint128_t)ts2 + (uint128_t)accd;

  // rounding
  // INT_ROUNDING(result, xrm, sew - 1);

  if (result > uint_max)
    overflow = true;
  // max saturation
  if (overflow) {
    result = uint_max;
    P_SET_OV(1);
  }

  accd = result;
}, USIGN, 1, e64)