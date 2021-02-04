require_rv64;

if (insn.rd() == X_SP)
    p->check_sp_update_value(sext32(RS1 + RS2));
WRITE_RD(sext32(RS1 + RS2));
