#include "blas_ops.h"
#include <iostream>

using namespace std;

typedef Stride<Dynamic, Dynamic> dStride_t;

#define MY_MATRIX_DEFINE(Type)                                                \
typedef Matrix<Type, Dynamic, Dynamic, RowMajor> Matrix_##Type;               \
typedef Map<Matrix_##Type, Unaligned, Stride<Dynamic, Dynamic> > Map_##Type;

/* Matrix_half   Map_half */
MY_MATRIX_DEFINE(half)
/* Matrix_uint8_t     Map_uint8_t */
MY_MATRIX_DEFINE(uint8_t)
/* Matrix_uint16_t     Map_uint16_t */
MY_MATRIX_DEFINE(uint16_t)

#undef MY_MATRIX_DEFINE


blas_ops::blas_ops()
{
    debug = 0;
    //cout << "Object is being created" << endl;
}

blas_ops::~blas_ops()
{
    //cout << "Object is being deleted" << endl;
}

void blas_ops::shape_stride_debug(struct shape_stride *ss)
{
    if (!debug)
        return;

    printf("\nshape_stride:\n");
    printf("shape1: (%d:%d)\n", ss->shape1_row, ss->shape1_column);
    printf("shape2: (%d:%d)\n", ss->shape2_row, ss->shape2_column);
    printf("stride rs1: %d\n", ss->stride_rs1);
    printf("stride rs2: %d\n", ss->stride_rs2);
    printf("stride rd : %d\n\n", ss->stride_rd);
}

int blas_ops::vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct shape_stride *ss)
{
    Map_uint8_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rd, 1));
    
    shape_stride_debug(ss);
    rd_matrix = rs1_matrix.cast<half>();

    return 0;
}

int blas_ops::veemul_mf(half *rs1, half *rd, half f, struct shape_stride *ss)
{
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    shape_stride_debug(ss);
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rd, 1));

    rd_matrix = rs1_matrix * f;

    return 0;
}

int blas_ops::veadd_mf(half *rs1, half *rd, half f, struct shape_stride *ss)
{
    /* param check */
    if ((rs1 == rd) && (ss->stride_rs1 != ss->stride_rd)) {
        cout << __FUNCTION__ << ": when rs1 equal rs2, stride_rs1 must equal stride_rd" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rd, 1));

    shape_stride_debug(ss);

    /* eigen not support matrix + scalar, so we creat a matrix init to const f, to
     * convert this operation to matrix + matrix
     */
    Matrix_half const_matrix(ss->shape1_row, ss->shape1_column);
    const_matrix = const_matrix.Constant(ss->shape1_row, ss->shape1_column, f);
    rd_matrix = rs1_matrix + const_matrix;

    return 0;
}

int blas_ops::vemul_mm(half *rs1, half *rs2, half *rd, struct shape_stride *ss)
{
    /* param check */
    if (ss->shape1_column != ss->shape2_row) {
        cout << __FUNCTION__ << ": shape1_column must equal shape2_row" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rs2_matrix(rs2, ss->shape2_row, ss->shape2_column, dStride_t(ss->stride_rs2, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape2_column, dStride_t(ss->stride_rd, 1));

    shape_stride_debug(ss);

    /* dot only support vector not support matrix, so we use '*' to do calculation */
    rd_matrix = rs1_matrix * rs2_matrix;
    return 0;
}

int blas_ops::vemul_mv(half *rs1, half *rs2, half *rd, struct shape_stride *ss)
{
    /* param check */
    if (ss->shape2_column != 1 && ss->shape2_row != 1) {
        cout << __FUNCTION__ << ": rs2 need a vector" << endl;
        return -BR_EPARAM;
    }

    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rs2_vector(rs2, ss->shape2_row, ss->shape2_column, dStride_t(ss->stride_rs2, 1));
    Map_half rd_vector(rd, ss->shape1_row, ss->shape2_column, dStride_t(ss->stride_rd, 1));

    shape_stride_debug(ss);
    
    /* dot only support vector not support matrix, so we use '*' to do calculation */
    rd_vector = rs1_matrix * rs2_vector;
    return 0;
}

int blas_ops::veacc_m(half *rs1, half *rd, struct shape_stride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rd_col_sum(rd, 1, ss->shape1_column, dStride_t(1, 1));
    Map_half rd_row_sum(rd, ss->shape1_row, 1, dStride_t(1, 1));
    
    switch (dim) {
    case 0: // column dir
        rd_col_sum = rs1_matrix.colwise().sum();
        break;
    case 1: // row dir
        rd_row_sum = rs1_matrix.rowwise().sum();
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

int blas_ops::veacc_m(half *rs1, half *rd, struct shape_stride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));

    *rd = rs1_matrix.sum();
    return 0;
}

int blas_ops::vemax_m(half *rs1, half *rd, struct shape_stride *ss, int dim)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rd_col_max(rd, 1, ss->shape1_column, dStride_t(1, 1));
    Map_half rd_row_max(rd, ss->shape1_row, 1, dStride_t(1, 1));
    
    switch (dim) {
    case 0: // column dir
        rd_col_max = rs1_matrix.colwise().maxCoeff();
        break;
    case 1: // row dir
        rd_row_max = rs1_matrix.rowwise().maxCoeff();
        break;
    default:
        cout << __FUNCTION__ << "error dim" << endl;
        return -BR_EPARAM;
    }
    return 0;
}

int blas_ops::velkrelu_mf(half *rs1, half rs2, half *rd, struct shape_stride *ss)
{
    Map_half rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rd, 1));
    
    rd_matrix = (rs1_matrix.array() > (half)0).select(rs1_matrix, rs1_matrix * rs2);
    return 0;
}

int blas_ops::velut_m(uint16_t *rs1, uint64_t rs2, half *rd, struct shape_stride *ss)
{
    Map_uint16_t rs1_matrix(rs1, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rs1, 1));
    Map_half rd_matrix(rd, ss->shape1_row, ss->shape1_column, dStride_t(ss->stride_rd, 1));

    //rs2 = rs2 & 0x3ff;
    for (int i = 0; i < ss->shape1_row; i++)
        for (int j = 0; j < ss->shape1_column; j++)
            rd_matrix(i, j) = *(half *)(rs2 + rs1_matrix(i, j));

    return 0;
}
