require_extension('F');
require_fp;
MMU.store_uint16(RS1 + insn.s_imm(), f32_to_f16(f32(FRS2.v[0])).v);

