check_traps_icmov;

auto chipId = DST_CHIP_ID;
auto coreId = DST_CORE_ID;

auto dataSize = RS2;

auto src = (uint8_t*)MMU.get_phy_addr(RS1);
auto dst = (uint8_t*)p->get_sim()->local_addr_to_mem_by_id_cluster(zext_xlen(RD), coreId);
assert(dst != nullptr && src != nullptr);

memcpy(dst, src, dataSize);

