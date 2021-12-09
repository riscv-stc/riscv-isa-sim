int64_t MSB = insn.ade_bf_msb();

int64_t LSB = insn.ade_bf_lsb();
int64_t lsbp1 = LSB + 1;
int64_t lsbm1 = LSB - 1;
int64_t msbm1 = MSB - 1;

int64_t lenm1 = 0;
int64_t rd_val = 0;
int64_t rs1_val = 0;

rd_val = RD;
rs1_val = RS1;

if(MSB == 0)
{
    if(GET_BIT(rs1_val, 0))
    {
        set_bit(LSB, (unsigned long *)&rd_val);
        WRITE_RD(rd_val);
    }
    else
    {
        clear_bit(LSB, (unsigned long *)&rd_val);
        WRITE_RD(rd_val);
    }
    
    if(LSB < 63)
    {
        WRITE_RD(rd_val & ((1 << lsbp1) - 1));
    }

    if(LSB > 0)
    {
        if (LSB < 63)
            WRITE_RD(rd_val & (~(1 << lsbm1 + 1) + 1));
        else
            WRITE_RD(0);
    }
        
}
else if (MSB < LSB)
{
    lenm1 = LSB - MSB;
    if (lenm1 < 63)
    {      
        rd_val = (rd_val & ((1 << MSB) - 1)) | (rd_val & (~(1 << LSB + 1) + 1)) | ( (rs1_val & (1 << (lenm1 + 1) - 1)) << MSB);
        WRITE_RD(rd_val); 
    }
    else
    {
        WRITE_RD(rs1_val); 
    }

    if(LSB < 63)
    {
        WRITE_RD(rd_val & ((1 << lsbp1) - 1));
    }
    WRITE_RD(rd_val & ((1 << (~(msbm1 + 1))) + 1));
}
else
{
    lenm1 = MSB - LSB;
    if (lenm1 < 63)
    {
        if (MSB < 60)
            rd_val = (rd_val & (~(1 << lenm1) + 1)) | ((rs1_val & ((1 << (MSB + 1)) - 1)) >> LSB);
        else
            rd_val = (rd_val & (~(1 << lenm1) + 1)) | (rs1_val >> LSB);

        WRITE_RD(rd_val);
    }
    else
    {
        WRITE_RD(rs1_val);
    }
    
    if (lenm1 < 63)
    {
        if(GET_BIT(rs1_val, MSB))
        {
            WRITE_RD(rd_val | (~(1 << (lenm1 + 1)) + 1));
        }
        else
        {
            WRITE_RD(rd_val & ((1 << (lenm1 + 1)) - 1));
        }
    }   
    else
        return 0;
}