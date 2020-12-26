check_traps_icmov;

auto src_coreId = SRC_CORE_ID;
auto dst_coreId = DST_CORE_ID;
auto dataSize = RS2;

unsigned int core_id = p->get_csr(CSR_TID);

if (core_id == src_coreId) {
    auto src = (uint8_t*)MMU.get_phy_addr(RS1);
    auto dst = (uint8_t*)p->get_sim()->local_addr_to_mem_by_id_cluster(zext_xlen(RD), dst_coreId);
    assert(dst != nullptr && src != nullptr);

    memcpy(dst, src, dataSize);
}
