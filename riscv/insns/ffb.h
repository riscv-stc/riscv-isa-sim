
union test {
    int a;
    char b;
} c;

c.a = 1;

uint8 rs2_vall8 = RS2 & 0xFF;

uint64_t rs1_val = RS1;

bool match1 = ((rs1_val & 0xFF) == rs2_vall8);
bool match2 = (((rs1_val >> 8) & 0xFF) == rs2_vall8);
bool match3 = (((rs1_val >> 16) & 0xFF) == rs2_vall8);
bool match4 = (((rs1_val >> 24) & 0xFF) == rs2_vall8);
bool match5 = (((rs1_val >> 32) & 0xFF) == rs2_vall8);
bool match6 = (((rs1_val >> 40) & 0xFF) == rs2_vall8);
bool match7 = (((rs1_val >> 48) & 0xFF) == rs2_vall8);
bool match8 = (((rs1_val >> 56) & 0xFF) == rs2_vall8);

