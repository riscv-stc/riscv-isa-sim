require_extension('F');
require_rv64;
require_fp;
softfloat_roundingMode = RM;
WRITE_FRD(f32_to_bf16(f32(FRS2)));
set_fp_exceptions;