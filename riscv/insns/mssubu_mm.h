MXU_GENERAL_LOOP_BASE
bool sat = false;

switch (sew) {
  case e8: {
    MX_PARAMS(e8);
    td = sat_subu<uint8_t>(ts1, ts2, sat);
    break;
  }
  case e16: {
    MX_PARAMS(e16);
    td = sat_subu<uint16_t>(ts1, ts2, sat);
    break;
  }
  case e32: {
    MX_PARAMS(e32);
    td = sat_subu<uint32_t>(ts1, ts2, sat);
    break;
  }
  default: {
    MX_PARAMS(e64);
    td = sat_subu<uint64_t>(ts1, ts2, sat);
    break;
  }
}
P_SET_OM(sat);

MXU_LOOP_END