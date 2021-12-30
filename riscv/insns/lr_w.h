require_extension('A');
auto res = MMU.amo_load_int32(RS1, true);
MMU.acquire_load_reservation(RS1);
WRITE_RD(res);
