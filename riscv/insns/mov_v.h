require_extension('V');
check_ncp_vill_invalid()

uint8_t e_size = 2;

switch (MTE_DATA_TYPE_RS1)
{
    case 0:
    case 1:
    case 5:
    case 6:
        e_size = 2; 
        break;
    case 2:
    case 7:
    case 8:
        e_size = 4;
        break;
    case 3:
    case 4:
        e_size = 1;
        break;
    default: 
        break;
}

check_traps_mov_v(e_size);

class CustomInsns CusIns;
struct ShapeStride sst;
sst_fill(sst, e_size, e_size);
unsigned long rs1 = MMU.get_phy_addr(RS1);
unsigned long rd = MMU.get_phy_addr(RD);

MTE_DTYPE_DECODING_TO_TYPE({
    mov_v<dtype_vd>((dtype_vd*)rs1, (dtype_vd*)rd, &sst, DIM_DM);
})