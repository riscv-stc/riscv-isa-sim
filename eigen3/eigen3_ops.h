/**************************************************************
 * Note: Eigen3 Interface 
 * Ver: V1.0
 *
 * Interface:
 *      class: CustomInsns
 *          func: vecvt_hf_xu8_m
 *          func: veemul_mf
 *          func: veadd_mf
 *          func: vemul_mm
 *          func: vemul_mv
 *          func: veacc_m
 *          func: vemax_m
 *          func: velkrelu_mf
 *          func: velut_m
 *   
 *      class: Vfwcvt
 *          func: vfwcvt_f_xu_v
 * 
 *      class: Vfmul
 *          func: vfmul_vf
 * 
 *      class: Vfadd
 *          func: vfadd_vf
 * 
 *      class: Vfmerge
 *          func: vfmerge_vf
 * 
 *      class: Vext
 *          func: vext_x_v
 * 
 *      class: Vfma
 *          func: vfmacc_vf
 * 
 *      class: Vfmax
 *          func: vfmax_vf
 *          func: vfmax_vv
 * 
 * log:
 * 2019.05.08  hao.chen       add file
 *       
 *
***************************************************************
*/
#ifndef __EIGEN3_OPS_H__
#define __EIGEN3_OPS_H__

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>

using namespace Eigen;
using namespace std;

struct ShapeStride
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


class CustomInsns
{
private:
    /* data */
    void ShapeStride_debug(struct ShapeStride *ss);
public:
    int debug;

    CustomInsns();
    ~CustomInsns();

    int vecvt_hf_xu8_m(uint8_t *rs1, half *rd, struct ShapeStride *ss);
    
    int veemul_mf(half *rs1, half *rd, half f, struct ShapeStride *ss);
    
    int veadd_mf(half *rs1, half *rd, half f, struct ShapeStride *ss);

    int vemul_mm(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);
    int vemul_mv(half *rs1, half *rs2, half *rd, struct ShapeStride *ss);

    int veacc_m(half *rs1, half *rd, struct ShapeStride *ss);
    int veacc_m(half *rs1, half *rd, struct ShapeStride *ss, int dim);
    
    int vemax_m(half *rs1, half *rd, struct ShapeStride *ss, int dim);
    
    int velkrelu_mf(half *rs1, half rs2, half *rd, struct ShapeStride *ss);
    
    int velut_m(uint16_t *rs1, uint64_t rs2, half *rd, struct ShapeStride *ss);
};


class Vfwcvt
{
  public:
    int vfwcvt_f_xu_v(uint8_t *vs2, half *vd, int num);
};

template <typename Type>
class Vfmul
{
  public:
    typedef Map<Matrix<Type, 1, Dynamic>> VfmulVecMap;
    
    int vfmul_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VfmulVecMap vector_vs2(vs2, num);
        VfmulVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2 * rs1;
        return 0;
    }
};

template <typename Type>
class Vfadd
{
  public:
    typedef Map<Matrix<Type, 1, Dynamic>> VfaddVecMap;
    
    int vfadd_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VfaddVecMap vector_vs2(vs2, num);
        VfaddVecMap vector_vd(vd, num);
        
        vector_vd = vector_vs2.array() + rs1;
        return 0;
    }
};

template <typename TypeData, typename TypeMask>
class Vfmerge
{
  public:
    typedef Map<Matrix<TypeData, 1, Dynamic>> VfmergeDataVecMap;
    typedef Map<Matrix<TypeMask, 1, Dynamic>> VfmergeMaskVecMap;

    int vfmerge_vf(TypeData *vs2, TypeData rs1, TypeData *vd, TypeMask *v0, int num)
    {
        VfmergeDataVecMap vector_vs2(vs2, num);
        VfmergeDataVecMap vector_vd(vd, num);
        VfmergeMaskVecMap vector_v0(v0, num);
        Matrix<TypeMask, 1, Dynamic> new_v0(num);

        for (int i = 0; i < num; i++)
            new_v0(i) = vector_v0(i) & 0x1;
        vector_vd = new_v0.array().select(rs1, vector_vs2);
        return 0;
    }
};

template <typename Type>
class Vext
{
  public:
    typedef Map<Matrix<Type, 1, Dynamic>> VextVecMap;
    
    int vext_x_v(Type *vs2, Type *rd, uint16_t rs1, int num)
    {
        VextVecMap vector_vs2(vs2, num);
        
        if (rs1 >= num)
            *rd = 0;
        else
            *rd = vector_vs2(rs1);
        return 0;
    }
};

template <typename Type>
class Vfma
{
  public:
    typedef Map<Matrix<Type, 1, Dynamic>> VfmaVecMap;
    
    int vfmacc_vf(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VfmaVecMap vector_vs2(vs2, num);
        VfmaVecMap vector_vs1(vs1, num);
        VfmaVecMap vector_vd(vd, num);

        vector_vd = vector_vs2.array() * vector_vs1.array() + vector_vd.array();
        return 0;
    }
};

template <typename Type>
class Vfmax
{
  public:
    typedef Map<Matrix<Type, 1, Dynamic>> VfmaxVecMap;
    
    int vfmax_vf(Type *vs2, Type rs1, Type *vd, int num)
    {
        VfmaxVecMap vector_vs2(vs2, num);
        VfmaxVecMap vector_vd(vd, num);

        vector_vd = (vector_vs2.array() > rs1).select(vector_vs2, rs1);
        return 0;
    }

    int vfmax_vv(Type *vs2, Type *vs1, Type *vd, int num)
    {
        VfmaxVecMap vector_vs2(vs2, num);
        VfmaxVecMap vector_vs1(vs1, num);
        VfmaxVecMap vector_vd(vd, num);

        vector_vd = (vector_vs2.array() > vector_vs1.array()).select(vector_vs2, vector_vs1);
        return 0;
    }
};

#endif
