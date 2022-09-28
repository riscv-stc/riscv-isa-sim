
uint64_t rs1_val = RS1;
uint64_t rs2_val = RS2;

bool F1, F2, F3, F4, F5, F6, F7, F8, found;

union test {
    int a;
    char b;
} c;

c.a = 0x1;

F1 = ((rs1_val & 0xFF) == 0) || ((rs1_val & 0xFF) != (rs2_val & 0xFF));
F2 = (((rs1_val >> 8) & 0xFF) == 0) || (((rs1_val >> 8)& 0xFF) != ((rs2_val >> 8) & 0xFF));
F3 = (((rs1_val >> 16) & 0xFF) == 0) || (((rs1_val >> 16)& 0xFF) != ((rs2_val >> 16) & 0xFF));
F4 = (((rs1_val >> 24) & 0xFF) == 0) || (((rs1_val >> 24)& 0xFF) != ((rs2_val >> 24) & 0xFF));
F5 = (((rs1_val >> 32) & 0xFF) == 0) || (((rs1_val >> 32)& 0xFF) != ((rs2_val >> 32) & 0xFF));
F6 = (((rs1_val >> 40) & 0xFF) == 0) || (((rs1_val >> 40)& 0xFF) != ((rs2_val >> 40) & 0xFF));
F7 = (((rs1_val >> 48) & 0xFF) == 0) || (((rs1_val >> 48)& 0xFF) != ((rs2_val >> 48) & 0xFF));
F8 = (((rs1_val >> 56) & 0xFF) == 0) || (((rs1_val >> 56)& 0xFF) != ((rs2_val >> 56) & 0xFF));

found = F1 || F2 || F3 || F4 || F5 || F6 || F7 || F8;

    if (!found) 
        WRITE_RD(0);
    else if (c.b & 0x1)  //Litile Endian
    {
        if (F1) 
            WRITE_RD(-8);
        else if (F2)
            WRITE_RD(-7);
        else if (F3) 
            WRITE_RD(-6);
        else if (F4) 
            WRITE_RD(-5);
        else if (F5) 
            WRITE_RD(-4);
        else if (F6) 
            WRITE_RD(-3);
        else if (F7) 
            WRITE_RD(-2);
        else 
            WRITE_RD(-1);
    }
    else 
    { // “Big Endian”
        if (F8) 
            WRITE_RD(-8);
        else if (F7) 
            WRITE_RD(-7);
        else if (F6) 
            WRITE_RD(-6);
        else if (F5) 
            WRITE_RD(-5);
        else if (F4) 
            WRITE_RD(-4);
        else if (F3) 
            WRITE_RD(-3);
        else if (F2) 
            WRITE_RD(-2);
        else 
            WRITE_RD(-1);
    }