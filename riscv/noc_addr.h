#ifndef __NOC_ADDR_H__
#define __NOC_ADDR_H__

#include "mmio_plugin.h"

#define DDR_SIZE       (0xc0000000)
#define NOC_NPC0_BASE  (0xDC000000)
#define NOC_NPC1_BASE  (0xD8000000)
#define NOC_NPC2_BASE  (0xD4000000)
#define NOC_NPC3_BASE  (0xD4800000)
#define NOC_NPC4_BASE  (0xD5000000)
#define NOC_NPC5_BASE  (0xD5800000)
#define NOC_NPC6_BASE  (0xD8800000)
#define NOC_NPC7_BASE  (0xDC800000)
#define NOC_NPC8_BASE  (0xEC000000)
#define NOC_NPC9_BASE  (0xE8000000)
#define NOC_NPC10_BASE (0xE4000000)
#define NOC_NPC11_BASE (0xE4800000)
#define NOC_NPC12_BASE (0xE5000000)
#define NOC_NPC13_BASE (0xE5800000)
#define NOC_NPC14_BASE (0xE8800000)
#define NOC_NPC15_BASE (0xEC800000)

#define NOC_NPC16_BASE (0xC8000000)
#define NOC_NPC17_BASE (0xC8800000)
#define NOC_NPC18_BASE (0xCA000000)
#define NOC_NPC19_BASE (0xCA800000)
#define NOC_NPC20_BASE (0xCC000000)
#define NOC_NPC21_BASE (0xCC800000)
#define NOC_NPC22_BASE (0xCE000000)
#define NOC_NPC23_BASE (0xCE800000)
#define NOC_NPC24_BASE (0xF0000000)
#define NOC_NPC25_BASE (0xF0800000)
#define NOC_NPC26_BASE (0xF2000000)
#define NOC_NPC27_BASE (0xF2800000)
#define NOC_NPC28_BASE (0xF4000000)
#define NOC_NPC29_BASE (0xF4800000)
#define NOC_NPC30_BASE (0xF6000000)
#define NOC_NPC31_BASE (0xF6800000)

#define NPC_SYS_OFFET   (0x7f0000)

/* NPUV2 Hardware Synchronization Buffer (512KB)  begin at 0xd3e80000*/
#define HWSYNC_START         (0xd3e80000)
#define HWSYNC_SIZE          (0x00080000)

#define NPC_LOCAL_ADDR_START (0xc0000000)
#define NPC_LOCAL_REGIN_SIZE (0x800000)

#define SRAM_START           (0xD3D80000)   /* NPUV2 IRAM 512K */
#define SRAM_SIZE            (0x80000)

int which_npc(reg_t addr, reg_t *paddr);

/* PCIE_CTL_CFG (128KB) */
#define PCIE_CTL_CFG_BASE       0xe3000000
#define PCIE_CTL_CFG_SIZE       0x100000

#define BANK0_MISC      0xd3d70000
#define BANK1_MISC      0xe3d70000
#define BANK2_MISC      0xc1204000
#define BANK3_MISC      0xf9204000
#define BANK_MISC_SIZE  0x1000

#endif