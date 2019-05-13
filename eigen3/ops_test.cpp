#include "eigen3_ops.h"

#define PRINT_FUNC    printf("\n\n ====================== %s ======================\n\n\n", __FUNCTION__)
#define PRINT_SUB_FUNC(sf)   printf("<<<<<<<<<<<%s<<<<<<<<<<<\n", sf)

static void printf_half(half *h, int num)
{
    printf("result:\n");
    
    for (int i = 0; i < num; i++) {
        if (i > 0 && !(i % 4))
            printf("\n");
        printf("%f(0x%04x)    ", (float)h[i], h[i].x);
    }
    printf("\n\n");
}

static void printf_uint16(uint16_t *h, int num)
{
    printf("result:\n");
    
    for (int i = 0; i < num; i++) {
        if (i > 0 && !(i % 4))
            printf("\n");
        printf("0x%04x    ", h[i]);
    }
    printf("\n\n");
}

#define SET_SHAPESTRIDE(ss, r1, c1, r2, c2, s1, s2, sd)   \
    ss.shape1_row = r1; \
    ss.shape1_column = c1; \
    ss.shape2_row = r2; \
    ss.shape2_column = c2; \
    ss.stride_rs1 = s1; \
    ss.stride_rs2 = s2; \
    ss.stride_rd = sd;


static void test_veadd(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rs2[i] = (half)i;
        rd[i] = (half)0;
    }

    PRINT_SUB_FUNC("veadd_mf");
    SET_SHAPESTRIDE(ss, 2, 4, 0, 0, 4, 0, 8);
    ci.veadd_mf(rs1, rd, (half)3, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veadd_mm");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 2, 4, 0, 0, 4, 4, 8);
    ci.veadd_mm(rs1, rd, rs2, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veadd_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 2, 4, 0, 0, 4, 0, 8);
    ci.veadd_mv(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veadd_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 2, 4, 0, 0, 4, 0, 8);
    ci.veadd_mv(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);
}

static void test_vesub(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)(i*2);
        rs2[i] = (half)(-i);
        rd[i] = (half)0;
    }

    PRINT_SUB_FUNC("vesub_mf");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.vesub_mf(rs1, rd, (half)13, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vesub_mm");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.vesub_mm(rs1, rd, rs2, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vesub_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.vesub_mv(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vesub_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.vesub_mv(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);
}

static void test_veemul(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rs2[i] = (half)i;
        rd[i] = (half)0;
    }

    PRINT_SUB_FUNC("veemul_mf");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.veemul_mf(rs1, rd, (half)13, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemul_mm");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.veemul_mm(rs1, rd, rs2, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemul_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.veemul_mv(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemul_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.veemul_mv(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);
}

static void test_veemacc(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)(i / 8 + 1);
        rs2[i] = (half)(i / 5 + 1);
        rd[i] = (half)0;
    }

    PRINT_SUB_FUNC("veemacc_mm");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.veemacc_mm(rs1, rd, rs2, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mm dim = 0");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.veemacc_mm(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mm dim = 1");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.veemacc_mm(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.veemacc_mv(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.veemacc_mv(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);
}

static void test_vecvt_hf_xu8_m(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    uint8_t rs1[32];
    half rd[32];
    
    PRINT_FUNC;

    for (int i = 0; i < 32; i++) {
        rs1[i] = (uint8_t)1;
        rd[i] = (half)0;
    }

    SET_SHAPESTRIDE(ss, 2, 4, 0, 0, 4, 0, 8);
    ci.vecvt_hf_xu8_m(rs1, rd, &ss);
    printf_half(rd, 32);
}

static void test_vemul_mm(void)
{
    class CustomInsns ci;
    half rs1[32];
    half rs2[32];
    half rd[32];
    int i;
    struct ShapeStride ss;

    ci.debug = 1;
    cout << endl << endl << ">>>>test_vemul_mm<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rs2[i] = (half)i;
        rd[i] = (half)0;
    }

    ss.shape1_row = 2;
    ss.shape1_column = 4;
    ss.shape2_row = 4;
    ss.shape2_column = 3;
    ss.stride_rs1 = 4;
    ss.stride_rs2 = 3;
    ss.stride_rd = 5;

    ci.vemul_mm(rs1, rs2, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

static void test_vemul_mv(void)
{
    class CustomInsns ci;
    half rs1[32];
    half rs2[32];
    half rd[32];
    int i;
    struct ShapeStride ss;

    ci.debug = 1;
    cout << endl << endl << ">>>>test_vemul_mv<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rs2[i] = (half)i;
        rd[i] = (half)0;
    }

    ss.shape1_row = 2;
    ss.shape1_column = 4;
    ss.shape2_row = 4;
    ss.shape2_column = 1;
    ss.stride_rs1 = 4;
    ss.stride_rs2 = 1;
    ss.stride_rd = 1;

    ci.vemul_mv(rs1, rs2, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

int test_veacc_m(void)
{
    class CustomInsns ci;
    half rs1[32];
    half rd[32];
    int i;
    struct ShapeStride ss;

    ci.debug = 1;
    cout << endl << endl << ">>>>test_veacc_m<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rd[i] = (half)0;
    }

    ss.shape1_row = 5;
    ss.shape1_column = 6;
    ss.stride_rs1 = 6;

    /* sum without dim */
    cout << "sum without dim" << endl;
    ci.veacc_m(rs1, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");

    /* sum with dim0 */
    for (i = 0; i < 32; i++)
        rd[i] = (half)0;
    cout << "sum with dim0" << endl;
    ci.veacc_m(rs1, rd, &ss, 0);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");

    /* sum with dim1 */
    for (i = 0; i < 32; i++)
        rd[i] = (half)0;
    cout << "sum with dim1" << endl;
    ci.veacc_m(rs1, rd, &ss, 1);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

static void test_vemax(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)(32 - i);
        rs2[i] = (half)(i + 2);
        rd[i] = (half)0;
    }

    PRINT_SUB_FUNC("vemax_mm");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.vemax_mm(rs1, rd, rs2, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemax_m");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.vemax_m(rs1, rd, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemax_m dim = 0");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.vemax_m(rs1, rd, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemax_m dim = 1");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.vemax_m(rs1, rd, &ss, 1);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemax_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.vemax_mv(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemax_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.vemax_mv(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);
}


static void test_vemin(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)(32 - i);
        rs2[i] = (half)(i + 2);
        rd[i] = (half)0;
    }

    PRINT_SUB_FUNC("vemin_mm");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.vemin_mm(rs1, rd, rs2, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemin_m");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.vemin_m(rs1, rd, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemin_m dim = 0");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.vemin_m(rs1, rd, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemin_m dim = 1");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.vemin_m(rs1, rd, &ss, 1);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemin_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.vemin_mv(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vemin_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    ci.vemin_mv(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);
}

static void test_velkrelu(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)(i - 15);
        rs2[i] = (half)(i + 2);
        rd[i] = (half)0;
    }

    PRINT_SUB_FUNC("velkrelu_mf");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 6, 6);
    ci.velkrelu_mf(rs1, rs2[0], rd, &ss);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("velkrelu_mv dim = 0");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.velkrelu_mv(rs1, rd, rs2, &ss, 0);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("velkrelu_mv dim = 1");
    SET_SHAPESTRIDE(ss, 5, 6, 0, 0, 6, 0, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.velkrelu_mv(rs1, rd, rs2, &ss, 1);
    printf_half(rd, 32);
}

int test_velut_m(void)
{
    class CustomInsns ci;
    uint16_t rs1[32];
    half base[32];
    half rd[32];
    int i;
    struct ShapeStride ss;

    ci.debug = 1;
    cout << endl << endl << ">>>>test_velut_m<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = i*2;
        base[i] = (half)(i + 100);
        rd[i] = (half)0;
    }

    ss.shape1_row = 5;
    ss.shape1_column = 6;
    ss.stride_rs1 = 6;
    ss.stride_rd = 6;

    ci.velut_m(rs1, (unsigned long)base, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

void test_vfwcvt_f_xu_v(void)
{
    half vd[32];
    uint8_t vs2[32];

    Vfwcvt myvi;
    cout << endl << endl << ">>>>test_vfwcvt_f_xu_v<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)0;
        vs2[i] = i;
    }

    myvi.vfwcvt_f_xu_v(vs2, vd, 32);
    
    cout << "result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("0x%x  ", vd[i].x);
    printf("\n\n");
}

void test_vfmul_vf(void)
{
    half vd[32];
    half vs2[32];
    half rs1 = (half)2.0;

    Vmul<half> myvi;
    cout << endl << endl << ">>>>test_vfmul_vf<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)0;
        vs2[i] = (half)i;
    }

    myvi.vmul_vf(vs2, rs1, vd, 32);
    
    cout << "result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("%f(0x%x)  ", (float)vd[i], vd[i].x);
    printf("\n\n");
}

void test_vfmerge_vf(void)
{
    half vd[32];
    half vs2[32];
    uint8_t v0[32];
    half rs1 = (half)99.0;

    Vmerge<half, uint8_t> myvi;
    cout << endl << endl << ">>>>test_vfmul_vf<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)0;
        vs2[i] = (half)i;
        v0[i] = i;
    }

    myvi.vmerge_vf(vs2, rs1, vd, 0, v0, 32);
    
    cout << "result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("%f(0x%x)  ", (float)vd[i], vd[i].x);
    printf("\n\n");
}

int test_vfmacc_vf(void)
{
    half vd[32];
    half vs2[32];
    half vs1[32];

    Vma<half> myvi;
    cout << endl << endl << ">>>>test_vfmacc_vf<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)10.0;
        vs2[i] = (half)3.0;
        vs1[i] = (half)2.0;
    }

    myvi.vmacc_vf(vs2, vs1[0], vd, 32);
    
    cout << "result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("0x%x  ", vd[i].x);
    printf("\n\n");
}

int test_vfmax(void)
{
    half vd[32];
    half vs2[32];
    half vs1[32];

    Vma<half> myvi;
    cout << endl << endl << ">>>>test_vfmax<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)0.0;
        vs2[i] = (half)i;
        vs1[i] = (half)15.0;
    }

    myvi.vmax_vf(vs2, vs1[0], vd, 32);
    
    cout << "vfmax_vf result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("%f  ", (float)vd[i]);
    printf("\n\n");

    for (int i = 0; i < 32; i++)
        vd[i] = (half)0.0;

    myvi.vmax_vv(vs2, vs1, vd, 32);
    cout << "vfmax_vv result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("%f  ", (float)vd[i]);
    printf("\n\n");
}

void test_vsgnj(void)
{
    class Vsgnj<half> ci;
    half vs1[32];
    half vs2[32];
    half vd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs1[i] = (half)(i - 15);
        vs2[i] = (half)(i - 10);
        vd[i] = (half)0;
    }

    PRINT_SUB_FUNC("vsgnj_vv");
    ci.vsgnj_vv(vs2, vs1, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnj_vf rs < 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnj_vf(vs2, (half)-5.0, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnj_vf rs > 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnj_vf(vs2, (half)5.0, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnjn_vv");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnjn_vv(vs2, vs1, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnjn_vf rs < 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnjn_vf(vs2, (half)-5.0, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnjn_vf rs > 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnjn_vf(vs2, (half)5.0, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnjx_vv");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnjx_vv(vs2, vs1, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnjx_vf rs < 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnjx_vf(vs2, (half)-5.0, vd, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsgnjx_vf rs > 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)0;
    ci.vsgnjx_vf(vs2, (half)5.0, vd, 32);
    printf_half(vd, 32);
}

void test_vcompare(void)
{
    class Vcompare<half, uint16_t> ci;
    half vs1[32];
    half vs2[32];
    uint16_t vd[32];
    
    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs1[i] = (half)i;
        vs2[i] = (half)i;
        vd[i] = (uint16_t)0;
    }
    for (int i = 10; i < 20; i++)
        vs2[i] = (half)1.0;

    PRINT_SUB_FUNC("veq_vv");
    ci.veq_vv(vs2, vs1, vd, 32);
    printf_uint16(vd, 32);

    PRINT_SUB_FUNC("vsgnj_vf");
    for (int i = 0; i < 32; i++)
        vd[i] = (uint16_t)0;
    ci.veq_vf(vs2, (half)5.0, vd, 32);
    printf_uint16(vd, 32);    
}

int main(void)
{
    /* custom insns */
    test_veadd();
    test_vesub();
    test_veemul();
    test_veemacc();
    test_vemax();
    test_vemin();
    test_velkrelu();

    /* vector insns */
    test_vsgnj();
    test_vcompare();

    test_vecvt_hf_xu8_m();
    test_vemul_mm();
    test_vemul_mv();
    test_veacc_m();
    test_velut_m();

    /* vector insns */
    test_vfwcvt_f_xu_v();
    test_vfmul_vf();
    test_vfmerge_vf();
    test_vfmacc_vf();
    test_vfmax();
    
    return 0;
}