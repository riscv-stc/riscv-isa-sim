require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
unsigned long rd = MMU.get_phy_addr(RD);
uint8_t e_size = 2;

switch (VME_DTYPE)
{
    case 0:
    {
        e_size = 2;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        Eigen::half vs1(f32(FRS1));
        mov_f(vs1, (Eigen::half*)rd, &sst);
        break;
    }
    case 0x10101:
    {
        e_size = 2;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        Eigen::Bfloat16 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Bfloat16*)rd, &sst);
        break;
    }
    case 0x20202:
    {
        e_size = 4;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        Eigen::Float32 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Float32*)rd, &sst);
        break;
    }
    default: 
        break;
}
