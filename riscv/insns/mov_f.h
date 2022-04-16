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
        WRITE_MEM_STC(RD, (half*)rd, CMT_LOG_VME_COM);
        break;
    }
    case 0x01:
    {
        e_size = 2;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        Eigen::Bfloat16 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Bfloat16*)rd, &sst);
        WRITE_MEM_STC(RD, (Bfloat16*)rd, CMT_LOG_VME_COM);
        break;
    }
    case 0x02:
    {
        e_size = 4;
        check_traps_mov_f(e_size);
        sst_fill(sst, 1, 1);
        Eigen::Float32 vs1(f32(FRS1));
        mov_f(vs1, (Eigen::Float32*)rd, &sst);
        WRITE_MEM_STC(RD, (Float32*)rd, CMT_LOG_VME_COM);
        break;
    }
    default: 
        break;
}
