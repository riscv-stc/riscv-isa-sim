check_traps_icmov;

auto srccoreId = SRC_CORE_ID;
auto dstcoreId = DST_CORE_ID;

auto dataSize = RS2;

auto src = (uint8_t*)MMU.get_phy_addr(RS1);
auto dst = (uint8_t*)p->get_sim()->local_addr_to_mem_by_id_cluster(zext_xlen(RD), dstcoreId);
assert(dst != nullptr && src != nullptr);

memcpy(dst, src, dataSize);

