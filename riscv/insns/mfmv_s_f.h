require_extension('D');
require_fp;
MMV_TR_FPR({
    ;
}, {
    ts1 = f16(FRS1);
},{
    ts1 = f32(FRS1);
},{
    ts1 = f64(FRS1);
}
,insn.rd());