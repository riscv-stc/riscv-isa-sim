#define boundary_index (32-1)

int32_t MSB = insn.ade_bf_msb();
int32_t LSB = insn.ade_bf_lsb();
int32_t lsbp1 = LSB + 1;
int32_t lsbm1 = LSB - 1;
int32_t msbm1 = MSB - 1;

int32_t lenm1 = 0;
uint32_t rd_val = 0;
uint32_t rs1_val = 0;

rd_val = RD;
rs1_val = RS1;

if(MSB == 0)
{
    if(GET_BIT(rs1_val, 0))
        set_bit(LSB, (unsigned long *)&rd_val);
    else
        clear_bit(LSB, (unsigned long *)&rd_val);

    
    if(LSB < boundary_index)
    {
        rd_val &= ((1U << lsbp1) - 1);
    }

    if(LSB > 0)
    {
        rd_val &= (~(1U << (lsbm1 + 1)) + 1);  
    }
     WRITE_RD(rd_val);
}
else if (MSB < LSB)
{
    lenm1 = LSB - MSB;
    if (LSB == boundary_index)
    {
        rd_val = (rd_val & ((1U << MSB) - 1)) ;
        rs1_val = (rs1_val & ((1U << (lenm1 + 1)) - 1)) ;
        rd_val |= (rs1_val << MSB);
    }
    else
    {
        rd_val = (rd_val & ((1U << MSB) - 1)) | (rd_val & (~(1U << (LSB + 1)) + 1)) | ( (rs1_val & ((1U << (lenm1 + 1)) - 1)) << MSB);

        if(LSB < boundary_index)
        {
            rd_val &= ((1U << lsbp1) - 1);
        }
    }
    rd_val &= ( (~(1U << (msbm1 + 1))) + 1);
    WRITE_RD(rd_val); 
}
else
{
    lenm1 = MSB - LSB;
    rd_val = 0;
    if ((MSB == boundary_index) && (LSB == 0))  
    {
        WRITE_RD(rs1_val);
    }
    else
    {
        rd_val =  ((rs1_val >> LSB ) & ((1U << (lenm1 + 1)) - 1)) ;
        WRITE_RD(rd_val);
    }

}