require_extension('C');
require_rv64;

if (insn.rvc_rd() == X_SP)
    p->check_sp_update_value(sext32(RVC_RS1S + RVC_RS2S));
WRITE_RVC_RS1S(sext32(RVC_RS1S + RVC_RS2S));
