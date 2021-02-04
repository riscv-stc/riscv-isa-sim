require_extension('C');

if (insn.rvc_rs1() == X_SP)
    p->check_sp_update_value(sext_xlen(RVC_RS1 + insn.rvc_imm()));
WRITE_RD(sext_xlen(RVC_RS1 + insn.rvc_imm()));
