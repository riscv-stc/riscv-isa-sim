
union test {
    int a;
    char b;
} c;

c.a = 0x1;

uint8_t rs2_vall8 = RS2 & 0xFF;

uint32_t rs1_val = RS1;

bool match1 = ((rs1_val & 0xFF) == rs2_vall8);
bool match2 = (((rs1_val >> 8) & 0xFF) == rs2_vall8);
bool match3 = (((rs1_val >> 16) & 0xFF) == rs2_vall8);
bool match4 = (((rs1_val >> 24) & 0xFF) == rs2_vall8);

bool found = match1 || match2 || match3 || match4;

    if (!found)
        WRITE_RD(0);
    else if(c.b & 0x1) //Little Endian
    {
        if(match1)
            WRITE_RD(-4);
        else if(match2)
            WRITE_RD(-3);
        else if(match3)
            WRITE_RD(-2);
        else 
            WRITE_RD(-1);
    }
    else{    //Big Endian      
        if(match4)
            WRITE_RD(-4);
        else if(match3)
            WRITE_RD(-3);
        else if(match2)
            WRITE_RD(-2);
        else 
            WRITE_RD(-1); 
    }