MX_NCVT(
{
    accd = P.MU.msew == e8 ? (j % 2 ? (accd & 0xF) | (acc1 & 0xF << 4) : (acc1 & 0xF)): acc1;
}, SUDU_NARR, 2, P.MU.msew)