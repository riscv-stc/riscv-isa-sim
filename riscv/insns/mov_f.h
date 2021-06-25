require_extension('V');

class CustomInsns CusIns;
struct ShapeStride sst;
unsigned long rd = MMU.get_phy_addr(RD);
uint8_t e_size = 2;

switch (MTE_DATA_TYPE_RS1)
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
    case 0x101:
    {
        e_size = 2;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        Eigen::Bfloat16 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Bfloat16*)rd, &sst);
        break;
    }
    case 0x202:
    {
        e_size = 4;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        Eigen::Float32 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Float32*)rd, &sst);
        break;
    }
    case 0x303:
    {
        e_size = 1;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        int8_t vs1 = bf16_to_i8(f32_to_bf16(f32(FRS1)), softfloat_round_near_maxMag, false);
        mov_f(vs1, (int8_t*)rd, &sst);
        break;
    }
    default: 
        break;
}
