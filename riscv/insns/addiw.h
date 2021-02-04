require_rv64;

if (insn.rs1() == X_SP)
    p->check_sp_update_value(sext32(RS1 + insn.i_imm()));
WRITE_RD(sext32(insn.i_imm() + RS1));
