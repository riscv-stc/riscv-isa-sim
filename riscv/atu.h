#ifndef _RISCV_ATU_H
#define _RISCV_ATU_H

#include "decode.h"
#include <cstdlib>
#include <string>
#include <vector>
#include <stdlib.h>
#include "devices.h"
#include "iniparser.h"

#define ENTRYX_PA_BASE_L_OFFSET     0x00
#define ENTRYX_PA_BASE_U_OFFSET     0x04
#define ENTRYX_IPA_END_L_OFFSET     0x08
#define ENTRYX_IPA_END_U_OFFSET     0x0c
#define EXTRYX_IPA_START_L_OFFSET   0x10
#define EXTRYX_IPA_START_U_OFFSET   0x14

#define IPA_ENTRYX_SIZE             0x14
#define IPA_ENTRY_TOTAL             16

#define ENTRY_IPA_EN_OFFSET         0x608
#define AT_CTL_REG_OFFSET           0x6f0

#define VFCFG_BIT_VF_EN         0
#define VFCFG_BIT_AT_EN         4
#define VFCFG_BIT_VF_ID         16
#define VFCFG_BIT_LOGI_CID      20
#define VFCFG_BIT_PHY_CID_BASE  24
#define VFCFG_BIT_NR_CLUSTERS   28

#define VFCFG_VF_ID_MASK            0x000f0000
#define VFCFG_VF_LOGI_CID_MASK      0x00f00000
#define VFCFG_VF_PHY_CID_BASE_MASK  0x0f000000
#define VFCFG_VF_NR_CLUSTERS_MASK   0xf0000000

#define PA_BASE_ADDR(at_base, entry_id)    ((*(uint64_t *)((char *)at_base+entry_id*IPA_ENTRYX_SIZE+ENTRYX_PA_BASE_U_OFFSET)&0xff)<<32 \
            | *(uint32_t *)((char *)at_base+entry_id*IPA_ENTRYX_SIZE+ENTRYX_PA_BASE_L_OFFSET))

#define IPA_START_ADDR(at_base, entry_id)    ((*(uint64_t *)((char *)at_base+entry_id*IPA_ENTRYX_SIZE+EXTRYX_IPA_START_U_OFFSET)&0xff)<<32 \
            | *(uint32_t *)((char *)at_base+entry_id*IPA_ENTRYX_SIZE+EXTRYX_IPA_START_L_OFFSET))

#define IPA_START_END(at_base, entry_id)    ((*(uint64_t *)((char *)at_base+entry_id*IPA_ENTRYX_SIZE+ENTRYX_IPA_END_U_OFFSET)&0xff)<<32 \
            | *(uint32_t *)((char *)at_base+entry_id*IPA_ENTRYX_SIZE+ENTRYX_IPA_END_L_OFFSET))

#define IS_ENTRY_ENABLE(at_base, entry_id)     ((*(uint32_t *)((char *)at_base+ENTRY_IPA_EN_OFFSET)>>entry_id) & 0x01)

#define IS_AT_ENABLE(at_base)               (*(uint32_t *)((char *)at_base+AT_CTL_REG_OFFSET) & (1<<VFCFG_BIT_AT_EN))

#define IPA_INVALID_ADDR            (~((reg_t)0))

struct ipa_at_t {
    reg_t pa_base;
    reg_t ipa_start;        /* 包含该地址 */
    reg_t ipa_end;          /* 不包含该地址 */
};

enum atu_type_t {
    NP_ATU,
    SYSDMA_ATU,
    MTE_ATU
};

class atu_t : public abstract_device_t
{
public:
    atu_t(const char *atuini, int procid, enum atu_type_t type);      /* npc */
    atu_t(const char *atuini, int dma_id, int ch_id, uint8_t *reg_base);    /* sysdma */
    ~atu_t();

    bool pmp_ok(reg_t addr, reg_t len) const;
    reg_t translate(reg_t addr, reg_t len) const;

    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);
    size_t size(void) {return len;};
    void reset(void);
    bool is_ipa_enabled(void) const {return at_enabled;};
    void set_atu_size(uint32_t size) {len = size;}   /* 修改atu 寄存器空间的size */

private:
    int procid;
    vector<struct ipa_at_t> ipa_at;
    dictionary *atini = nullptr;
    bool at_enabled = false;
    int ipa_entry_max = IPA_ENTRY_TOTAL;
    uint8_t *at_reg_base = nullptr;
    size_t len = 4096;
    enum atu_type_t atu_type;

    /* sysdma atu */
    int dma_id;
    int ch_id;

    int at_update(uint8_t *at_base);
    int at_update(dictionary *ini, int procid);
    int at_update(dictionary *ini, int dma_id, int ch_id);

    /* 寄存器操作接口,操作寄存器后 at_update() 才能生效 */
    int reg_add_at(struct ipa_at_t *at, int entry_id, uint32_t *at_base);
    int reg_at_enable(bool enabled, uint32_t *at_base);
    int reg_at_entry_enable(int entry_id, bool enabled, uint32_t *at_base);
};

#define IDTU_VF_CTRL        0x00

#define IS_IDTU_ENABLE(reg_base)           (*(uint32_t *)((char *)reg_base+IDTU_VF_CTRL) & (1<<VFCFG_BIT_VF_EN))

class idtu_t : public abstract_device_t
{
public:
    idtu_t(void);
    ~idtu_t();

    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);
    size_t size(void) {return sizeof(reg_base);};
    void reset(void);

    uint32_t idtu_coreid_trans(uint32_t logi_core_id);    /* icmov */
    uint32_t idtu_coremap_trans(uint32_t logi_coremap);   /* pld */

private:
    uint8_t reg_base[MTE_IOV_IDTU_SIZE];
};


#endif
