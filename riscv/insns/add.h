if (insn.rd() == X_SP)
    p->check_sp_update_value(sext_xlen(RS1 + RS2));
WRITE_RD(sext_xlen(RS1 + RS2));
