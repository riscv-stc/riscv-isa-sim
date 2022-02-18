// See LICENSE for license details.

#ifndef _RISCV_BANKIF_H
#define _RISCV_BANKIF_H

#include "decode.h"
#include <iostream>

enum local_device_type {
  L1_BUFFER,
  IM_BUFFER,
  IO_DEVICE,
  SP_BUFFER,
};

class bank_t;
class processor_t;
// this is the interface to the simulator used by the processors and memory
class bankif_t
{
public:
    virtual int get_bankid(void) = 0;
    virtual bool is_bank_finish(void) = 0;
    virtual void set_bank_finish(bool finish) = 0;

    virtual bool is_bottom_ddr(reg_t addr) const = 0;
    virtual reg_t bottom_ddr_to_upper(reg_t addr) const = 0;
    virtual bool in_npc_mem(reg_t addr, local_device_type type) = 0;

    virtual char* bank_addr_to_mem(reg_t addr) = 0;
    virtual char* npc_addr_to_mem(reg_t addr, uint32_t idxinbank) = 0;

    virtual bool bank_mmio_load(reg_t addr, size_t len, uint8_t* bytes) = 0;
    virtual bool bank_mmio_store(reg_t addr, size_t len, const uint8_t* bytes) = 0;

    virtual bool npc_mmio_load(reg_t addr, size_t len, uint8_t* bytes, uint32_t idxinbank) = 0;
    virtual bool npc_mmio_store(reg_t addr, size_t len, const uint8_t* bytes, uint32_t idxinbank) = 0;
};

#endif
