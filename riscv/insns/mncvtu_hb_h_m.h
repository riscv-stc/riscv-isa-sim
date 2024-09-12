MX_NCVT_QUAD(
{
    accd = j % 2 ? (accd & 0xF) | (acc1 & 0xF << 4) : (acc1 & 0xF);
}, SUDU_NARR, 4, e16)