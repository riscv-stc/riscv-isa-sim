if (insn.rs1() == X_SP)
    p->check_sp_update_value(sext_xlen(RS1 + insn.i_imm()));
WRITE_RD(sext_xlen(RS1 + insn.i_imm()));
