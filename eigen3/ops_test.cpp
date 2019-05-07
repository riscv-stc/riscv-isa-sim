#include "blas_ops.h"
#include <iostream>

using namespace std;

static void test_veadd_mf(void)
{
    class blas_ops myops;
    half rs1[32];
    half rd[32];
    int i;
    struct shape_stride ss;

    cout << endl << endl << ">>>>test_veadd_mf<<<<" << endl;
    myops.debug = 1;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i;
        rd[i] = (half)0;
    }

    ss.shape1_row = 2;
    ss.shape1_column = 4;
    ss.stride_rs1 = 5;
    ss.stride_rd = 8;

    myops.veadd_mf(rs1, rd, (half)3, &ss);
    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f    ", (float)rd[i]);
    printf("\n\n");
}

static void test_vecvt_hf_xu8_m(void)
{
    class blas_ops myops;
    uint8_t rs1[32];
    half rd[32];
    int i;
    struct shape_stride ss;
    
    myops.debug = 1;
    cout << endl << endl << ">>>>test_vecvt_hf_xu8_m<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (uint8_t)i;
        rd[i] = (half)0;
    }

    ss.shape1_row = 2;
    ss.shape1_column = 4;
    ss.stride_rs1 = 4;
    ss.stride_rd = 8;

    myops.vecvt_hf_xu8_m(rs1, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

static void test_vemul_mm(void)
{
    class blas_ops myops;
    half rs1[32];
    half rs2[32];
    half rd[32];
    int i;
    struct shape_stride ss;

    myops.debug = 1;
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

    myops.vemul_mm(rs1, rs2, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

static void test_vemul_mv(void)
{
    class blas_ops myops;
    half rs1[32];
    half rs2[32];
    half rd[32];
    int i;
    struct shape_stride ss;

    myops.debug = 1;
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

    myops.vemul_mv(rs1, rs2, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

int test_veacc_m(void)
{
    class blas_ops myops;
    half rs1[32];
    half rd[32];
    int i;
    struct shape_stride ss;

    myops.debug = 1;
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
    myops.veacc_m(rs1, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");

    /* sum with dim0 */
    for (i = 0; i < 32; i++)
        rd[i] = (half)0;
    cout << "sum with dim0" << endl;
    myops.veacc_m(rs1, rd, &ss, 0);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");

    /* sum with dim1 */
    for (i = 0; i < 32; i++)
        rd[i] = (half)0;
    cout << "sum with dim1" << endl;
    myops.veacc_m(rs1, rd, &ss, 1);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

int test_vemax_m(void)
{
    class blas_ops myops;
    half rs1[32];
    half rd[32];
    int i;
    struct shape_stride ss;

    myops.debug = 1;
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
    myops.vemax_m(rs1, rd, &ss, 0);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");

    /* sum with dim1 */
    for (i = 0; i < 32; i++)
        rd[i] = (half)0;
    cout << "max with dim1" << endl;
    myops.vemax_m(rs1, rd, &ss, 1);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

int test_velkrelu_mf(void)
{
    class blas_ops myops;
    half rs1[32];
    half rd[32];
    int i;
    struct shape_stride ss;

    myops.debug = 1;
    cout << endl << endl << ">>>>test_velkrelu_mf<<<<" << endl;

    for (i = 0; i < 32; i++) {
        rs1[i] = (half)i + (half)-10;
        rd[i] = (half)0;
    }

    ss.shape1_row = 5;
    ss.shape1_column = 6;
    ss.stride_rs1 = 6;
    ss.stride_rd = 6;

    myops.velkrelu_mf(rs1, (half)2, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
    printf("\n\n");
}

int test_velut_m(void)
{
    class blas_ops myops;
    uint16_t rs1[32];
    half base[32];
    half rd[32];
    int i;
    struct shape_stride ss;

    myops.debug = 1;
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

    myops.velut_m(rs1, (uint64_t)base, rd, &ss);

    cout << "result is:" << endl;
    for (i = 0; i < 32; i++)
        printf("%f  ", (float)rd[i]);
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

    return 0;
}