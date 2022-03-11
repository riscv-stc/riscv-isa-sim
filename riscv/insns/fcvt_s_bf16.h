require_extension('F');
require_rv64;
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD(bf16_to_f32(bf16(FRS2)));
set_fp_exceptions;