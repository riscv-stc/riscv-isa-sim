require_extension('F');
require_fp;
float16_t f16;
assert(insn.i_imm() == 0);
f16.v = MMU.load_uint16(RS1);
WRITE_FRD(f16_to_f32(f16));
