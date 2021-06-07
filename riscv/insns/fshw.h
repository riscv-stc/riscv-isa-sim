require_extension('F');
require_fp;
assert(insn.s_imm() == 0);
MMU.store_uint16(RS1, f32_to_f16(f32(FRS2.v[0])).v);
