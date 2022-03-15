
union test {
    int a;
    char b;
} c;

c.a = 0x1;

uint8_t rs2_vall8 = RS2 & 0xFF;

uint64_t rs1_val = RS1;

bool match1 = ((rs1_val & 0xFF) == rs2_vall8);
bool match2 = (((rs1_val >> 8) & 0xFF) == rs2_vall8);
bool match3 = (((rs1_val >> 16) & 0xFF) == rs2_vall8);
bool match4 = (((rs1_val >> 24) & 0xFF) == rs2_vall8);
bool match5 = (((rs1_val >> 32) & 0xFF) == rs2_vall8);
bool match6 = (((rs1_val >> 40) & 0xFF) == rs2_vall8);
bool match7 = (((rs1_val >> 48) & 0xFF) == rs2_vall8);
bool match8 = (((rs1_val >> 56) & 0xFF) == rs2_vall8);

bool found = match1 || match2 || match3 || match4 || match5 || match6 || \
             match7 || match8;

    if (!found)
        WRITE_RD(0);
    else if(c.b & 0x1) //Little Endian
    {
        if(match1)
            WRITE_RD(-8);
        else if(match2)
            WRITE_RD(-7);
        else if(match3)
            WRITE_RD(-6);
        else if(match4)
            WRITE_RD(-5);
        else if(match5)
            WRITE_RD(-4);
        else if(match6)
            WRITE_RD(-3);
        else if(match7)
            WRITE_RD(-2);
        else 
            WRITE_RD(-1);
    }
    else{    //Big Endian
        
        if(match8)
            WRITE_RD(-8);
        else if(match7)
            WRITE_RD(-7);
        else if(match6)
            WRITE_RD(-6);
        else if(match5)
            WRITE_RD(-5);
        else if(match4)
            WRITE_RD(-4);
        else if(match3)
            WRITE_RD(-3);
        else if(match2)
            WRITE_RD(-2);
        else 
            WRITE_RD(-1);
        
    }