#include "eigen3_ops.h"
#include <iostream>
#include <fstream>

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


#define TEST_OPEN_FILE(func)                \
    string fpath = "./dataset/";            \
    fpath += func;                          \
    fpath += "_data.h";                     \
    ofstream ofs(fpath, ios::out);          \
    if (!ofs) {                             \
        cerr << fpath << "fail" << endl;    \
        return;                             \
    }


#define TEST_ADD_MATRIX(name, pdata, rows, cols, len)               \
    string matrix##name = "\nstatic int16_t ";                      \
    matrix##name += #name;                                          \
    matrix##name += "[";                                            \
    matrix##name += #len;                                           \
    matrix##name += "] = {\n";                                      \
    ofs << matrix##name;                                            \
    for (int row = 0; row < rows; row++) {                          \
        ofs << "    ";                                              \
        for (int col = 0; col < cols; col++)                        \
            ofs << "0x" << hex << pdata[col + row*cols].x << ", ";  \
        ofs << endl;                                                \
    }                                                               \
    ofs << "};\n\n";

#define TEST_ADD_S(name, data)                     \
    string s_##name = "\nstatic int32_t ";         \
    s_##name += #name;                             \
    s_##name += "[1] = {\n";                       \
    ofs << s_##name;                               \
    ofs << "    0x" << hex << data.x << "\n};\n"


#define TEST_COMB_POINTER(pname, name)   \
    string comb##pname = "\np_";         \
    comb##pname += #pname;               \
    comb##pname += " = (void *)";        \
    comb##pname += #name;                \
    comb##pname += ";\n";                \
    ofs << comb##pname;

#define TEST_FILL_SHAPE(number, row, column)  \
    string shape##number = "\nshape";         \
    shape##number += #number;                 \
    shape##number += " = DEFINE_SHAPE(";      \
    shape##number += #column;                 \
    shape##number += ", ";                    \
    shape##number += #row;                    \
    shape##number += ");\n";                  \
    ofs << shape##number;

/* dest stride */
#define TEST_FILL_STRIDE1(sd)                           \
    string stride1 = "\nstride1 = DEFINE_STRIDE(0, ";   \
    stride1 += #sd;                                     \
    stride1 += ");\n";                                  \
    ofs << stride1;

/**
 * @breif fill stride2
 * @param s2 源矩阵2 stride
 * @param s1 源矩阵1 stride
 */
#define TEST_FILL_STRIDE2(s2, s1)                            \
    string stride##number = "\nstride2 = DEFINE_STRIDE(";    \
    stride##number += #s2;                                   \
    stride##number += ", ";                                  \
    stride##number += #s1;                                   \
    stride##number += ");\n";                                \
    ofs << stride##number;

#define TEST_CLOSE_FILE     \
    ofs << endl << endl;    \
    ofs.close();

static void test_veadd(void)
{
    class CustomInsns ci;
    struct ShapeStride ss;
    half rs1[32];
    half rs2[32];
    half rd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        rs1[i] = (half)(rand() % 10 + 2);
        rs2[i] = (half)(rand() % 10 + 5);
        rd[i] = (half)0;
    }

    TEST_OPEN_FILE("veadd");

    /* export data to file */
    TEST_ADD_MATRIX(rs1_m, rs1, 4, 8, 32);
    TEST_ADD_MATRIX(rs2_m, rs2, 4, 7, 28);
    TEST_ADD_MATRIX(rs2_dim0_v, rs2, 4, 1, 4);
    TEST_ADD_MATRIX(rs2_dim1_v, rs2, 1, 6, 6);
    TEST_ADD_S(rs2_s, rs2[0]);

    PRINT_SUB_FUNC("veadd_mf");
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veadd_mf(rs1, rd, rs2[0], &ss);
    TEST_ADD_MATRIX(golden_mf_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veadd_mm");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 7, 6);
    ci.veadd_mm(rs1, rd, rs2, &ss);
    TEST_ADD_MATRIX(golden_mm_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veadd_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veadd_mv(rs1, rd, rs2, &ss, 0);
    TEST_ADD_MATRIX(golden_mv_dim0_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veadd_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veadd_mv(rs1, rd, rs2, &ss, 1);
    TEST_ADD_MATRIX(golden_mv_dim1_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    TEST_ADD_MATRIX(rd_m, rd, 0, 0, 24);

    TEST_COMB_POINTER(rs1_m, rs1_m);
    TEST_COMB_POINTER(rs2_m, rs2_m);
    TEST_COMB_POINTER(rs2_dim0_v, rs2_dim0_v);
    TEST_COMB_POINTER(rs2_dim1_v, rs2_dim1_v);
    TEST_COMB_POINTER(rs2_s, rs2_s);
    TEST_COMB_POINTER(rd_m, rd_m);
    TEST_FILL_SHAPE(1, 4, 6);
    TEST_FILL_SHAPE(2, 4, 6);
    TEST_FILL_STRIDE2(7, 8);
    TEST_FILL_STRIDE1(6);
    TEST_CLOSE_FILE;
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
        rs1[i] = (half)(rand() % 10 + 5);
        rs2[i] = (half)(rand() % 10 + 2);
        rd[i] = (half)0;
    }

    TEST_OPEN_FILE("vesub");

    /* export data to file */
    TEST_ADD_MATRIX(rs1_m, rs1, 4, 8, 32);
    TEST_ADD_MATRIX(rs2_m, rs2, 4, 7, 28);
    TEST_ADD_MATRIX(rs2_dim0_v, rs2, 4, 1, 4);
    TEST_ADD_MATRIX(rs2_dim1_v, rs2, 1, 6, 6);
    TEST_ADD_S(rs2_s, rs2[0]);

    PRINT_SUB_FUNC("vesub_mf");
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.vesub_mf(rs1, rd, rs2[0], &ss);
    TEST_ADD_MATRIX(golden_mf_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vesub_mm");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 7, 6);
    ci.vesub_mm(rs1, rd, rs2, &ss);
    TEST_ADD_MATRIX(golden_mm_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vesub_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.vesub_mv(rs1, rd, rs2, &ss, 0);
    TEST_ADD_MATRIX(golden_mv_dim0_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("vesub_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.vesub_mv(rs1, rd, rs2, &ss, 1);
    TEST_ADD_MATRIX(golden_mv_dim1_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    TEST_ADD_MATRIX(rd_m, rd, 0, 0, 24);

    TEST_COMB_POINTER(rs1_m, rs1_m);
    TEST_COMB_POINTER(rs2_m, rs2_m);
    TEST_COMB_POINTER(rs2_dim0_v, rs2_dim0_v);
    TEST_COMB_POINTER(rs2_dim1_v, rs2_dim1_v);
    TEST_COMB_POINTER(rs2_s, rs2_s);
    TEST_COMB_POINTER(rd_m, rd_m);
    TEST_FILL_SHAPE(1, 4, 6);
    TEST_FILL_SHAPE(2, 4, 6);
    TEST_FILL_STRIDE2(7, 8);
    TEST_FILL_STRIDE1(6);
    TEST_CLOSE_FILE;
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
        rs1[i] = (half)(rand() % 10 + 2);
        rs2[i] = (half)(rand() % 10 + 5);
        rd[i] = (half)0;
    }

    TEST_OPEN_FILE("veemul");

    /* export data to file */
    TEST_ADD_MATRIX(rs1_m, rs1, 4, 8, 32);
    TEST_ADD_MATRIX(rs2_m, rs2, 4, 7, 28);
    TEST_ADD_MATRIX(rs2_dim0_v, rs2, 4, 1, 4);
    TEST_ADD_MATRIX(rs2_dim1_v, rs2, 1, 6, 6);
    TEST_ADD_S(rs2_s, rs2[0]);

    PRINT_SUB_FUNC("veemul_mf");
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veemul_mf(rs1, rd, rs2[0], &ss);
    TEST_ADD_MATRIX(golden_mf_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemul_mm");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 7, 6);
    ci.veemul_mm(rs1, rd, rs2, &ss);
    TEST_ADD_MATRIX(golden_mm_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemul_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veemul_mv(rs1, rd, rs2, &ss, 0);
    TEST_ADD_MATRIX(golden_mv_dim0_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemul_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veemul_mv(rs1, rd, rs2, &ss, 1);
    TEST_ADD_MATRIX(golden_mv_dim1_m, rd, 4, 6, 24);
    printf_half(rd, 32);

    TEST_ADD_MATRIX(rd_m, rd, 0, 0, 24);

    TEST_COMB_POINTER(rs1_m, rs1_m);
    TEST_COMB_POINTER(rs2_m, rs2_m);
    TEST_COMB_POINTER(rs2_dim0_v, rs2_dim0_v);
    TEST_COMB_POINTER(rs2_dim1_v, rs2_dim1_v);
    TEST_COMB_POINTER(rs2_s, rs2_s);
    TEST_COMB_POINTER(rd_m, rd_m);
    TEST_FILL_SHAPE(1, 4, 6);
    TEST_FILL_SHAPE(2, 4, 6);
    TEST_FILL_STRIDE2(7, 8);
    TEST_FILL_STRIDE1(6);
    TEST_CLOSE_FILE;
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
        rs1[i] = (half)(rand() % 10 + 3);
        rs2[i] = (half)(rand() % 10 + 4);
        rd[i] = (half)0;
    }

    TEST_OPEN_FILE("veemacc");

    /* export data to file */
    TEST_ADD_MATRIX(rs1_m, rs1, 4, 8, 32);
    TEST_ADD_MATRIX(rs2_m, rs2, 4, 7, 28);
    TEST_ADD_MATRIX(rs2_dim0_v, rs2, 4, 1, 4);
    TEST_ADD_MATRIX(rs2_dim1_v, rs2, 1, 6, 6);

    PRINT_SUB_FUNC("veemacc_mm");
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 7, 6);
    ci.veemacc_mm(rs1, rd, rs2, &ss);
    TEST_ADD_MATRIX(golden_mm_s, rd, 1, 1, 1);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mm dim = 0");
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 7, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.veemacc_mm(rs1, rd, rs2, &ss, 0);
    TEST_ADD_MATRIX(golden_mm_dim0_v, rd, 1, 6, 6);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mm dim = 1");
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 7, 6);
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    ci.veemacc_mm(rs1, rd, rs2, &ss, 1);
    TEST_ADD_MATRIX(golden_mm_dim1_v, rd, 4, 1, 4);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mv dim = 0");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veemacc_mv(rs1, rd, rs2, &ss, 0);
    TEST_ADD_MATRIX(golden_mv_dim0_v, rd, 1, 6, 6);
    printf_half(rd, 32);

    PRINT_SUB_FUNC("veemacc_mv dim = 1");
    for (int i = 0; i < 32; i++)
        rd[i] = (half)0;
    SET_SHAPESTRIDE(ss, 4, 6, 0, 0, 8, 0, 6);
    ci.veemacc_mv(rs1, rd, rs2, &ss, 1);
    TEST_ADD_MATRIX(golden_mv_dim1_v, rd, 4, 1, 4);
    printf_half(rd, 32);

    TEST_ADD_MATRIX(rd_s, rd, 0, 0, 1);
    TEST_ADD_MATRIX(rd_v, rd, 0, 0, 32); // enough

    TEST_COMB_POINTER(rs1_m, rs1_m);
    TEST_COMB_POINTER(rs2_m, rs2_m);
    TEST_COMB_POINTER(rs2_dim0_v, rs2_dim0_v);
    TEST_COMB_POINTER(rs2_dim1_v, rs2_dim1_v);
    
    TEST_COMB_POINTER(rd_s, rd_s);
    TEST_COMB_POINTER(rd_v, rd_v);

    TEST_FILL_SHAPE(1, 4, 6);
    TEST_FILL_SHAPE(2, 4, 6);
    TEST_FILL_STRIDE2(7, 8);
    TEST_FILL_STRIDE1(6);
    TEST_CLOSE_FILE;
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

void test_vfwcvt(void)
{
    Vfwcvt<uint16_t> cvt;
    uint16_t vs2_ui[32];
    int16_t vs2_i[32];
    uint16_t v0[32];
    half vd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs2_ui[i] = (uint16_t)i;
        vs2_i[i] = (int16_t)(i - 15);
        v0[i] = i / 20;
        vd[i] = (half)99.0;
    }

    PRINT_SUB_FUNC("vfwcvt_f_x_v vm = 0");
    cvt.vfwcvt_f_x_v(vs2_i, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vfwcvt_f_x_v vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    cvt.vfwcvt_f_x_v(vs2_i, vd, 1, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vfwcvt_f_xu_v vm = 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    cvt.vfwcvt_f_xu_v(vs2_ui, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vfwcvt_f_xu_v vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    cvt.vfwcvt_f_xu_v(vs2_ui, vd, 1, v0, 32);
    printf_half(vd, 32);
}

void test_vfadd(void)
{
    Vadd<half, uint16_t> add;
    half vs2[32];
    half vs1[32];
    uint16_t v0[32];
    half vd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs2[i] = (half)(rand() % 10 + 2);
        vs1[i] = (half)(rand() % 10 + 5);
        v0[i] = i / 20;
        vd[i] = (half)99.0;
    }

    TEST_OPEN_FILE("vfadd");

    TEST_ADD_MATRIX(vs2_v, vs2, 1, 32, "32");
    TEST_ADD_MATRIX(vs1_v, vs1, 1, 32, "32");

    PRINT_SUB_FUNC("vadd_vf vm = 0");
    add.vadd_vf(vs2, (half)5.0, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vadd_vf vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    add.vadd_vf(vs2, (half)5.0, vd, 1, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vadd_vv vm = 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    add.vadd_vv(vs2, vs1, vd, 0, v0, 32);
    TEST_ADD_MATRIX(golden_vadd_vv, vd, 1, 32, "32");
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vadd_vv vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    add.vadd_vv(vs2, vs1, vd, 1, v0, 32);
    printf_half(vd, 32);

    TEST_CLOSE_FILE;
}

void test_vsub(void)
{
    Vsub<half, uint16_t> sub;
    half vs2[32];
    half vs1[32];
    uint16_t v0[32];
    half vd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs2[i] = (half)(i + 3);
        vs1[i] = (half)i;
        v0[i] = i / 20;
        vd[i] = (half)99.0;
    }

    PRINT_SUB_FUNC("vsub_vf vm = 0");
    sub.vsub_vf(vs2, (half)5.0, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsub_vf vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    sub.vsub_vf(vs2, (half)5.0, vd, 1, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsub_vv vm = 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    sub.vsub_vv(vs2, vs1, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vsub_vv vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    sub.vsub_vv(vs2, vs1, vd, 1, v0, 32);
    printf_half(vd, 32);
}

void test_vmul(void)
{
    Vmul<half, uint16_t> mul;
    half vs2[32];
    half vs1[32];
    uint16_t v0[32];
    half vd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs2[i] = (half)i;
        vs1[i] = (half)i;
        v0[i] = i / 20;
        vd[i] = (half)99.0;
    }

    PRINT_SUB_FUNC("vmul_vf vm = 0");
    mul.vmul_vf(vs2, (half)5.0, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vmul_vf vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    mul.vmul_vf(vs2, (half)5.0, vd, 1, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vmul_vv vm = 0");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    mul.vmul_vv(vs2, vs1, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vmul_vv vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    mul.vmul_vv(vs2, vs1, vd, 1, v0, 32);
    printf_half(vd, 32);
}

void test_vmerge(void)
{
    Vmerge<half, uint16_t> merge;
    half vs2[32];
    half vs1[32];
    uint16_t v0[32];
    half vd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs2[i] = (half)i;
        vs1[i] = (half)i;
        v0[i] = i / 20;
        vd[i] = (half)99.0;
    }

    PRINT_SUB_FUNC("vmerge_vf vm = 0");
    merge.vmerge_vf(vs2, (half)5.0, vd, 0, v0, 32);
    printf_half(vd, 32);

    PRINT_SUB_FUNC("vmerge_vf vm = 1");
    for (int i = 0; i < 32; i++)
        vd[i] = (half)99.0;
    merge.vmerge_vf(vs2, (half)5.0, vd, 1, v0, 32);
    printf_half(vd, 32);
}

void test_vsgnj(void)
{
    Vsgnj<half, uint16_t> sgnj;
    half vs1[32];
    half vs2[32];
    half vd[32];
    uint16_t v0[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        v0[i] = i / 20;
        vs1[i] = (half)(i - 15);
        vs2[i] = (half)(i - 10);
        vd[i] = (half)0;
    }

    #define VSGNJ_TEST_VV(func, func_str)          \
        PRINT_SUB_FUNC(func_str" vm = 0");         \
        for (int i = 0; i < 32; i++)               \
            vd[i] = (half)0;                       \
        sgnj.func(vs2, vs1, vd, 0, v0, 32);        \
        printf_half(vd, 32);                       \
                                                   \
        PRINT_SUB_FUNC(func_str" vm = 1");         \
        for (int i = 0; i < 32; i++)               \
            vd[i] = (half)0;                       \
        sgnj.func(vs2, vs1, vd, 1, v0, 32);        \
        printf_half(vd, 32);

    #define VSGNJ_TEST_VF(func, func_str)          \
        PRINT_SUB_FUNC(func_str" vm = 0 rs < 0");  \
        for (int i = 0; i < 32; i++)               \
            vd[i] = (half)0;                       \
        sgnj.func(vs2, (half)-5.0, vd, 0, v0, 32); \
        printf_half(vd, 32);                       \
                                                   \
        PRINT_SUB_FUNC(func_str" vm = 0 rs > 0");  \
        for (int i = 0; i < 32; i++)               \
            vd[i] = (half)0;                       \
        sgnj.func(vs2, (half)5.0, vd, 0, v0, 32);  \
        printf_half(vd, 32);                       \
                                                   \
        PRINT_SUB_FUNC(func_str" vm = 1 rs < 0");  \
        for (int i = 0; i < 32; i++)               \
            vd[i] = (half)0;                       \
        sgnj.func(vs2, (half)-5.0, vd, 1, v0, 32); \
        printf_half(vd, 32);                       \
                                                   \
        PRINT_SUB_FUNC(func_str" vm = 1 rs > 0");  \
        for (int i = 0; i < 32; i++)               \
            vd[i] = (half)0;                       \
        sgnj.func(vs2, (half)5.0, vd, 1, v0, 32);  \
        printf_half(vd, 32);

    VSGNJ_TEST_VV(vsgnj_vv, "vsgnj_vv");
    VSGNJ_TEST_VF(vsgnj_vf, "vsgnj_vf");

    VSGNJ_TEST_VV(vsgnjn_vv, "vsgnjn_vv");
    VSGNJ_TEST_VF(vsgnjn_vf, "vsgnjn_vf");

    VSGNJ_TEST_VV(vsgnjx_vv, "vsgnjx_vv");
    VSGNJ_TEST_VF(vsgnjx_vf, "vsgnjx_vf");
}

void test_vext(void)
{
    Vext<half> ext;
    half vs2[32];
    half rd;

    PRINT_FUNC;
    for (int i = 0; i < 32; i++)
        vs2[i] = (half)i;

    PRINT_SUB_FUNC("vext_x_v");
    ext.vext_x_v(vs2, &rd, (uint16_t)10, 32);

    printf("rd = %f(0x%04x)\n", (float)rd, rd.x);
}

void test_vma(void)
{
    Vma<half, uint16_t> ma;
    half vs2[32];
    half vs1[32];
    uint16_t v0[32];
    half vd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs2[i] = (half)(i + 6);
        vs1[i] = (half)(i + 5);
        v0[i] = i / 20;
        vd[i] = (half)2.0;
    }

    #define VMA_TEST_VF(func, func_str)                \
        PRINT_SUB_FUNC(func_str " vm = 0");            \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (half)2.0;                         \
        ma.func(vs2, (half)5.0, vd, 0, v0, 32);        \
        printf_half(vd, 32);                           \
                                                       \
        PRINT_SUB_FUNC(func_str" vm = 1");             \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (half)2.0;                         \
        ma.func(vs2, (half)5.0, vd, 1, v0, 32);        \
        printf_half(vd, 32);

    #define VMA_TEST_VV(func, func_str)                \
        PRINT_SUB_FUNC(func_str" vm = 0");             \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (half)2.0;                         \
        ma.func(vs2, vs1, vd, 0, v0, 32);              \
        printf_half(vd, 32);                           \
                                                       \
        PRINT_SUB_FUNC(func_str" vm = 1");             \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (half)2.0;                         \
        ma.func(vs2, vs1, vd, 1, v0, 32);              \
        printf_half(vd, 32);


    VMA_TEST_VF(vmacc_vf, "vmacc_vf");
    VMA_TEST_VV(vmacc_vv, "vmacc_vv");

    VMA_TEST_VF(vmadd_vf, "vmadd_vf");
    VMA_TEST_VV(vmadd_vv, "vmadd_vv");

    VMA_TEST_VF(vmax_vf, "vmax_vf");
    VMA_TEST_VV(vmax_vv, "vmax_vv");

    VMA_TEST_VF(vmin_vf, "vmin_vf");
    VMA_TEST_VV(vmin_vv, "vmin_vv");

    VMA_TEST_VF(vmsac_vf, "vmsac_vf");
    VMA_TEST_VV(vmsac_vv, "vmsac_vv");

    VMA_TEST_VF(vnmsub_vf, "vnmsub_vf");
    VMA_TEST_VV(vnmsub_vv, "vnmsub_vv");

    VMA_TEST_VF(vnmacc_vf, "vnmacc_vf");
    VMA_TEST_VV(vnmacc_vv, "vnmacc_vv");

    VMA_TEST_VF(vnmadd_vf, "vnmadd_vf");
    VMA_TEST_VV(vnmadd_vv, "vnmadd_vv");

    VMA_TEST_VF(vnmsac_vf, "vnmsac_vf");
    VMA_TEST_VV(vnmsac_vv, "vnmsac_vv");

    VMA_TEST_VF(vnmsub_vf, "vnmsub_vf");
    VMA_TEST_VV(vnmsub_vv, "vnmsub_vv");

    VMA_TEST_VF(vnmsub_vf, "vnmsub_vf");
    VMA_TEST_VV(vnmsub_vv, "vnmsub_vv");
}

void test_vcompare(void)
{
    Vcompare<half, uint16_t, uint16_t> cmp;
    half vs2[32];
    half vs1[32];
    uint16_t v0[32];
    uint16_t vd[32];

    PRINT_FUNC;
    for (int i = 0; i < 32; i++) {
        vs1[i] = (half)i;
        vs2[i] = (half)i;
        vd[i] = (uint16_t)0;
        v0[i] = i / 20;
    }
    for (int i = 10; i < 20; i++)
        vs2[i] = (half)1.0;

    #define VCMP_TEST_VF(func, func_str)               \
        PRINT_SUB_FUNC(func_str " vm = 0");            \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (uint16_t)3;                       \
        cmp.func(vs2, (half)15.0, vd, 0, v0, 32);      \
        printf_uint16(vd, 32);                         \
                                                       \
        PRINT_SUB_FUNC(func_str" vm = 1");             \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (uint16_t)3;                       \
        cmp.func(vs2, (half)15.0, vd, 1, v0, 32);      \
        printf_uint16(vd, 32);

    #define VCMP_TEST_VV(func, func_str)               \
        PRINT_SUB_FUNC(func_str" vm = 0");             \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (uint16_t)3;                       \
        cmp.func(vs2, vs1, vd, 0, v0, 32);             \
        printf_uint16(vd, 32);                         \
                                                       \
        PRINT_SUB_FUNC(func_str" vm = 1");             \
        for (int i = 0; i < 32; i++)                   \
            vd[i] = (uint16_t)3;                       \
        cmp.func(vs2, vs1, vd, 1, v0, 32);             \
        printf_uint16(vd, 32);

    VCMP_TEST_VF(veq_vf, "veq_vf");
    VCMP_TEST_VV(veq_vv, "veq_vv");

    VCMP_TEST_VF(vge_vf, "vge_vf");
    VCMP_TEST_VV(vge_vv, "vge_vv");

    VCMP_TEST_VF(vgt_vf, "vgt_vf");
    VCMP_TEST_VV(vgt_vv, "vgt_vv");

    VCMP_TEST_VF(vle_vf, "vle_vf");
    VCMP_TEST_VV(vle_vv, "vle_vv");

    VCMP_TEST_VF(vlt_vf, "vlt_vf");
    VCMP_TEST_VV(vlt_vv, "vlt_vv");

    VCMP_TEST_VF(vne_vf, "vne_vf");
    VCMP_TEST_VV(vne_vv, "vne_vv");
}

int main(void)
{
    /* custom insns */
    test_veadd();
    test_vesub();
    test_veemul();
    test_veemacc();
    /*
    test_vemax();
    test_vemin();
    test_velkrelu();

    test_vecvt_hf_xu8_m();
    test_vemul_mm();
    test_vemul_mv();
    test_veacc_m();
    test_velut_m();

    test_vfwcvt();
    test_vfadd();
    test_vsub();
    test_vmul();
    test_vmerge();
    test_vext();
    test_vma();
    test_vsgnj();
    test_vcompare();
    */
    return 0;
}
