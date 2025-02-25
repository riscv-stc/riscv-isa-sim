MX_NCVT_QUAD(
{
    accd = j % 2 == 0 ? (accd & 0xF) | ((acc1 & 0xF) << 4) : (acc1 & 0xF) | (accd & 0xF0);
}, SSDS_NARR, 4, e16)
