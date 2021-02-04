require_extension('C');
require(insn.rvc_addi4spn_imm() != 0);
p->check_sp_update_value(sext_xlen(RVC_SP + insn.rvc_addi4spn_imm()));
WRITE_RVC_RS2S(sext_xlen(RVC_SP + insn.rvc_addi4spn_imm()));
