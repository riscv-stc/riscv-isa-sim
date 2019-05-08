#include "eigen3_ops.h"

static void test_veadd_mf(void)
{
    class CustomInsns ci;
    half rs1[32];
    half rd[32];
    int i;
    struct ShapeStride ss;

    cout << endl << endl << ">>>>test_veadd_mf<<<<" << endl;
    ci.debug = 1;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rd[i] = (half)0;
    }

    ss.shape1_row = 2;
    ss.shape1_column = 4;
    ss.stride_rs1 = 5;
    ss.stride_rd = 8;

    ci.veadd_mf(rs1, rd, (half)3, &ss);
    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f    ", (float)rd[i]);
    printf("\n\n");
}

static void test_vecvt_hf_xu8_m(void)
{
    class CustomInsns ci;
    uint8_t rs1[32];
    half rd[32];
    int i;
    struct ShapeStride ss;
    
    ci.debug = 1;
    cout << endl << endl << ">>>>test_vecvt_hf_xu8_m<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (uint8_t)1;
        rd[i] = (half)0;
    }

    ss.shape1_row = 2;
    ss.shape1_column = 4;
    ss.stride_rs1 = 4;
    ss.stride_rd = 8;

    ci.vecvt_hf_xu8_m(rs1, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f(0x%x)  ", (float)rd[i], rd[i].x);
    printf("\n\n");
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

int test_vemax_m(void)
{
    class CustomInsns ci;
    half rs1[32];
    half rd[32];
    int i;
    struct ShapeStride ss;

    ci.debug = 1;
    cout << endl << endl << ">>>>test_vemax_m<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rd[i] = (half)0;
    }

    ss.shape1_row = 5;
    ss.shape1_column = 6;
    ss.stride_rs1 = 6;

    /* max with dim0 */
    cout << "max with dim0" << endl;
    ci.vemax_m(rs1, rd, &ss, 0);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");

    /* sum with dim1 */
    for (i = 0; i < 32; i++)
        rd[i] = (half)0;
    cout << "max with dim1" << endl;
    ci.vemax_m(rs1, rd, &ss, 1);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

int test_velkrelu_mf(void)
{
    class CustomInsns ci;
    half rs1[32];
    half rd[32];
    int i;
    struct ShapeStride ss;

    ci.debug = 1;
    cout << endl << endl << ">>>>test_velkrelu_mf<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i + (half)-10;
        rd[i] = (half)0;
    }

    ss.shape1_row = 5;
    ss.shape1_column = 6;
    ss.stride_rs1 = 6;
    ss.stride_rd = 6;

    ci.velkrelu_mf(rs1, (half)2, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
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

    ci.velut_m(rs1, (uint64_t)base, rd, &ss);

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

    Vfmul<half> myvi;
    cout << endl << endl << ">>>>test_vfmul_vf<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)0;
        vs2[i] = (half)i;
    }

    myvi.vfmul_vf(vs2, rs1, vd, 32);
    
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

    Vfmerge<half, uint8_t> myvi;
    cout << endl << endl << ">>>>test_vfmul_vf<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)0;
        vs2[i] = (half)i;
        v0[i] = i;
    }

    myvi.vfmerge_vf(vs2, rs1, vd, v0, 32);
    
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

    Vfma<half> myvi;
    cout << endl << endl << ">>>>test_vfmacc_vf<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)10.0;
        vs2[i] = (half)3.0;
        vs1[i] = (half)2.0;
    }

    myvi.vfmacc_vf(vs2, vs1, vd, 32);
    
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

    Vfmax<half> myvi;
    cout << endl << endl << ">>>>test_vfmax<<<<" << endl;

    for (int i = 0; i < 32; i++) {
        vd[i] = (half)0.0;
        vs2[i] = (half)i;
        vs1[i] = (half)15.0;
    }

    myvi.vfmax_vf(vs2, vs1[0], vd, 32);
    
    cout << "vfmax_vf result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("%f  ", (float)vd[i]);
    printf("\n\n");

    for (int i = 0; i < 32; i++)
        vd[i] = (half)0.0;

    myvi.vfmax_vv(vs2, vs1, vd, 32);
    cout << "vfmax_vv result is:" << endl;
    for (int i = 0; i < 32; i++)
        printf("%f  ", (float)vd[i]);
    printf("\n\n");
}


int main(void)
{
    test_veadd_mf();
    test_vecvt_hf_xu8_m();
    test_vemul_mm();
    test_vemul_mv();
    test_veacc_m();
    test_vemax_m();
    test_velkrelu_mf();
    test_velut_m();
    test_vfwcvt_f_xu_v();
    test_vfmul_vf();
    test_vfmerge_vf();
    test_vfmacc_vf();
    test_vfmax();
    
    return 0;
}