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


#define HWSYNC_START          (0xd0080000)

/* NPUV2 The lower 3GB region (0x00_0000_0000 ~ 0x00_BFFF_FFFF) is the remapping target region of DDR space */
#define ddr_mem_start        (0x00000000)

/* NPUV2 L1 Buffer (1024KB+288KB) */
#define l1_buffer_start      (0xc0000000)
#define l1_buffer_size       (0x00148000)

/* NPUV2 Intermediate Buffer(256KB) */
#define im_buffer_start      (0xc0400000)
#define im_buffer_size       (0x00040000)

/* NPUV2 Index(.sp) RAM (80KB) */
#define sp_buffer_start      (0xc0500000)
#define sp_buffer_size       (0x00014000)

#define MISC_START           (0xc07f3000)   /* NPUV2 NP_MISC 4KB */
#define MISC_SIZE            (0x1000)

#define MBOX_START           (0xc07f4000)   /* NPUV2 NP_MBOX_LOC 4KB */

#define NP_IOV_ATU_START    (0xc07f7000)    /* ipa address trans unit */
#define NP_IOV_ATU_SIZE     (0x1000)

#define NPC_LOCAL_ADDR_START (l1_buffer_start)
#define NPC_LOCAL_REGIN_SIZE (0x800000)

int which_npc(reg_t addr, reg_t *paddr);

#endif