MX_NCVT(
{
    accd = j % 2 ? (accd & 0xF) | (acc1 & 0xF << 4) : (acc1 & 0xF);
}, SSDS_NARR_E4, 1, e8)