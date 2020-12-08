require_extension('V');
check_ncp_vill_invalid()

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
        sst_fill(sst, e_size, e_size);
        Eigen::half vs1(f32(FRS1));
        mov_f(vs1, (Eigen::half*)rd, &sst);
        break;
    }
    case 1:
    {
        e_size = 2;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        Eigen::Bfloat16 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Bfloat16*)rd, &sst);
        break;
    }
    case 2:
    {
        e_size = 4;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        Eigen::Float32 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Float32*)rd, &sst);
        break;
    }
    case 3:
    {
        e_size = 1;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        int8_t vs1 = RS1 & 0xff;
        mov_f(vs1, (int8_t*)rd, &sst);
        break;
    }
    case 4:
    {
        e_size = 1;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        uint8_t vs1 = RS1 & 0xff;
        mov_f(vs1, (uint8_t*)rd, &sst);
        break;
    }
    case 5:
    {
        e_size = 2;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        int16_t vs1 = RS1 & 0xffff;
        mov_f(vs1, (int16_t*)rd, &sst);
        break;
    }
    case 6:
    {
        e_size = 2;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        uint16_t vs1 = RS1 & 0xffff;
        mov_f(vs1, (uint16_t*)rd, &sst);
        break;
    }
    case 7:
    {
        e_size = 4;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        int32_t vs1 = RS1 & 0xffffffff;
        mov_f(vs1, (int32_t*)rd, &sst);
        break;
    }
    case 8:
    {
        e_size = 4;
        check_traps_mov_f(e_size);
        sst_fill(sst, e_size, e_size);
        uint32_t vs1 = RS1 & 0xffffffff;;
        mov_f(vs1, (uint32_t*)rd, &sst);
        break;
    }
    default: 
        break;
}
