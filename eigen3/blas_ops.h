#ifndef __BLAS_OPS_H__
#define __BLAS_OPS_H__

#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace Eigen;

struct shape_stride
{
    /* CSR shape1 */
    unsigned short shape1_column;
    unsigned short shape1_row;

    /* CSR shape2 */
    unsigned short shape2_column;
    unsigned short shape2_row;
    
    /* CSR stride1 */
    unsigned short stride_rd;
    
    /* CSR stride2 */
    unsigned short stride_rs2;
    unsigned short stride_rs1;
};

enum {
    BR_OK = 0,
    BR_EPARAM
};


class blas_ops
{
private:
    /* data */
    void shape_stride_debug(struct shape_stride *ss);
public:
    int debug;

    blas_ops();
    ~blas_ops();

    int vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct shape_stride *ss);
    
    int veemul_mf(half *rs1, half *rd, half f, struct shape_stride *ss);
    
    int veadd_mf(half *rs1, half *rd, half f, struct shape_stride *ss);
    
    int vemul_mm(half *rs1, half *rs2, half *rd, struct shape_stride *ss);
    int vemul_mv(half *rs1, half *rs2, half *rd, struct shape_stride *ss);

    int veacc_m(half *rs1, half *rd, struct shape_stride *ss);
    int veacc_m(half *rs1, half *rd, struct shape_stride *ss, int dim);
    
    int vemax_m(half *rs1, half *rd, struct shape_stride *ss, int dim);
    
    int velkrelu_mf(half *rs1, half rs2, half *rd, struct shape_stride *ss);
    
    int velut_m(uint16_t *rs1, uint64_t rs2, half *rd, struct shape_stride *ss);
};

#endif
