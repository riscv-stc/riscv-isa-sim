int64_t MSB = insn.ade_bf_msb();

int64_t LSB = insn.ade_bf_lsb();
int64_t lsbp1 = LSB + 1;
int64_t lsbm1 = LSB - 1;
int64_t msbm1 = MSB - 1;

int64_t lenm1 = 0;
uint64_t rd_val = 0;
uint64_t rs1_val = 0;

rd_val = RD;
rs1_val = RS1;

if(MSB == 0)
{
    if(GET_BIT(rs1_val, 0))
        set_bit(LSB, (unsigned long *)&rd_val);
    else
        clear_bit(LSB, (unsigned long *)&rd_val);

    
    if(LSB < 63)
    {
        if (rs1_val & 0x1)
            rd_val |= ((~(1UL << lsbp1 )) + 1);
        else
            rd_val &= ((1UL << lsbp1) - 1);
    }

    if(LSB > 0)
    {
        rd_val &= (~(1UL << (lsbm1 + 1)) + 1);  
    }
     WRITE_RD(rd_val);
}
else if (MSB < LSB)
{
    lenm1 = LSB - MSB;
    if (LSB == 63)
    {
        rd_val = (rd_val & ((1UL << MSB) - 1)) ;
        rs1_val = (rs1_val & ((1UL << (lenm1 + 1)) - 1)) ;
        rd_val |= (rs1_val << MSB);
    }
    else
    {
        rd_val = (rd_val & ((1UL << MSB) - 1)) | (rd_val & (~(1UL << (LSB + 1)) + 1)) | ( (rs1_val & ((1UL << (lenm1 + 1)) - 1)) << MSB);

        if(LSB < 63)
        {
            if ((rs1_val >> lenm1) & 0x1)
                rd_val |= ((~(1UL << lsbp1)) + 1);
            else
                rd_val &= ((1UL << lsbp1) - 1);
        }
    }
    rd_val &= ( (~(1UL << (msbm1 + 1))) + 1);
    WRITE_RD(rd_val); 
}
else
{
    lenm1 = MSB - LSB;
    rd_val = 0;
    if ((MSB == 63) && (LSB == 0))  
    {
        WRITE_RD(rs1_val);
    }
    else
    {
        if ((rs1_val >> MSB) & 0x1)
        {
            rd_val =  ((rs1_val >> LSB ) | ((~(1UL << (lenm1 + 1))) + 1));
        }
        else
        {
            rd_val =  ((rs1_val >> LSB ) & ((1UL << (lenm1 + 1)) - 1)) ;
        }
        WRITE_RD(rd_val);
    }

}