require_extension('C');
require(insn.rvc_rs2() != 0);

if (insn.rvc_rd() == X_SP)
    p->check_sp_update_value(sext_xlen(RVC_RS1 + RVC_RS2));
WRITE_RD(sext_xlen(RVC_RS1 + RVC_RS2));
