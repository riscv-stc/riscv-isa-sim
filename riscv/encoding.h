/* See LICENSE for license details. */

#ifndef RISCV_CSR_ENCODING_H
#define RISCV_CSR_ENCODING_H

#define MSTATUS_UIE         0x00000001
#define MSTATUS_SIE         0x00000002
#define MSTATUS_HIE         0x00000004
#define MSTATUS_MIE         0x00000008
#define MSTATUS_UPIE        0x00000010
#define MSTATUS_SPIE        0x00000020
#define MSTATUS_UBE         0x00000040
#define MSTATUS_MPIE        0x00000080
#define MSTATUS_SPP         0x00000100
#define MSTATUS_VS          0x00000600
#define MSTATUS_MPP         0x00001800
#define MSTATUS_FS          0x00006000
#define MSTATUS_XS          0x00018000
#define MSTATUS_MPRV        0x00020000
#define MSTATUS_SUM         0x00040000
#define MSTATUS_MXR         0x00080000
#define MSTATUS_TVM         0x00100000
#define MSTATUS_TW          0x00200000
#define MSTATUS_TSR         0x00400000
#define MSTATUS32_SD        0x80000000
#define MSTATUS_UXL         0x0000000300000000
#define MSTATUS_SXL         0x0000000C00000000
#define MSTATUS_SBE         0x0000001000000000
#define MSTATUS_MBE         0x0000002000000000
#define MSTATUS_GVA         0x0000004000000000
#define MSTATUS_MPV         0x0000008000000000
#define MSTATUS64_SD        0x8000000000000000

#define MSTATUSH_SBE        0x00000010
#define MSTATUSH_MBE        0x00000020

#define SSTATUS_UIE         0x00000001
#define SSTATUS_SIE         0x00000002
#define SSTATUS_UPIE        0x00000010
#define SSTATUS_SPIE        0x00000020
#define SSTATUS_UBE         0x00000040
#define SSTATUS_SPP         0x00000100
#define SSTATUS_VS          0x00000600
#define SSTATUS_FS          0x00006000
#define SSTATUS_XS          0x00018000
#define SSTATUS_SUM         0x00040000
#define SSTATUS_MXR         0x00080000
#define SSTATUS32_SD        0x80000000
#define SSTATUS_UXL         0x0000000300000000
#define SSTATUS64_SD        0x8000000000000000

#define SSTATUS_VS_MASK     (SSTATUS_SIE | SSTATUS_SPIE | \
                             SSTATUS_SPP | SSTATUS_FS | SSTATUS_SUM | \
                             SSTATUS_MXR | SSTATUS_UXL)

#define HSTATUS_VSXL        0x300000000
#define HSTATUS_VTSR        0x00400000
#define HSTATUS_VTW         0x00200000
#define HSTATUS_VTVM        0x00100000
#define HSTATUS_VGEIN       0x0003f000
#define HSTATUS_HU          0x00000200
#define HSTATUS_SPVP        0x00000100
#define HSTATUS_SPV         0x00000080
#define HSTATUS_GVA         0x00000040
#define HSTATUS_VSBE        0x00000020

#define USTATUS_UIE         0x00000001
#define USTATUS_UPIE        0x00000010

#define DCSR_XDEBUGVER      (3U<<30)
#define DCSR_NDRESET        (1<<29)
#define DCSR_FULLRESET      (1<<28)
#define DCSR_EBREAKM        (1<<15)
#define DCSR_EBREAKH        (1<<14)
#define DCSR_EBREAKS        (1<<13)
#define DCSR_EBREAKU        (1<<12)
#define DCSR_STOPCYCLE      (1<<10)
#define DCSR_STOPTIME       (1<<9)
#define DCSR_CAUSE          (7<<6)
#define DCSR_DEBUGINT       (1<<5)
#define DCSR_HALT           (1<<3)
#define DCSR_STEP           (1<<2)
#define DCSR_PRV            (3<<0)

#define DCSR_CAUSE_NONE     0
#define DCSR_CAUSE_SWBP     1
#define DCSR_CAUSE_HWBP     2
#define DCSR_CAUSE_DEBUGINT 3
#define DCSR_CAUSE_STEP     4
#define DCSR_CAUSE_HALT     5
#define DCSR_CAUSE_GROUP    6

#define MCONTROL_TYPE(xlen)    (0xfULL<<((xlen)-4))
#define MCONTROL_DMODE(xlen)   (1ULL<<((xlen)-5))
#define MCONTROL_MASKMAX(xlen) (0x3fULL<<((xlen)-11))

#define MCONTROL_SELECT     (1<<19)
#define MCONTROL_TIMING     (1<<18)
#define MCONTROL_ACTION     (0x3f<<12)
#define MCONTROL_CHAIN      (1<<11)
#define MCONTROL_MATCH      (0xf<<7)
#define MCONTROL_M          (1<<6)
#define MCONTROL_H          (1<<5)
#define MCONTROL_S          (1<<4)
#define MCONTROL_U          (1<<3)
#define MCONTROL_EXECUTE    (1<<2)
#define MCONTROL_STORE      (1<<1)
#define MCONTROL_LOAD       (1<<0)

#define MCONTROL_TYPE_NONE      0
#define MCONTROL_TYPE_MATCH     2

#define MCONTROL_ACTION_DEBUG_EXCEPTION   0
#define MCONTROL_ACTION_DEBUG_MODE        1
#define MCONTROL_ACTION_TRACE_START       2
#define MCONTROL_ACTION_TRACE_STOP        3
#define MCONTROL_ACTION_TRACE_EMIT        4

#define MCONTROL_MATCH_EQUAL     0
#define MCONTROL_MATCH_NAPOT     1
#define MCONTROL_MATCH_GE        2
#define MCONTROL_MATCH_LT        3
#define MCONTROL_MATCH_MASK_LOW  4
#define MCONTROL_MATCH_MASK_HIGH 5

#define MIP_USIP            (1 << IRQ_U_SOFT)
#define MIP_SSIP            (1 << IRQ_S_SOFT)
#define MIP_VSSIP           (1 << IRQ_VS_SOFT)
#define MIP_MSIP            (1 << IRQ_M_SOFT)
#define MIP_UTIP            (1 << IRQ_U_TIMER)
#define MIP_STIP            (1 << IRQ_S_TIMER)
#define MIP_VSTIP           (1 << IRQ_VS_TIMER)
#define MIP_MTIP            (1 << IRQ_M_TIMER)
#define MIP_UEIP            (1 << IRQ_U_EXT)
#define MIP_SEIP            (1 << IRQ_S_EXT)
#define MIP_VSEIP           (1 << IRQ_VS_EXT)
#define MIP_MEIP            (1 << IRQ_M_EXT)
#define MIP_SGEIP           (1 << IRQ_S_GEXT)

#define MIP_S_MASK          (MIP_SSIP | MIP_STIP | MIP_SEIP)
#define MIP_VS_MASK         (MIP_VSSIP | MIP_VSTIP | MIP_VSEIP)
#define MIP_HS_MASK         (MIP_VS_MASK | MIP_SGEIP)

#define MIDELEG_FORCED_MASK MIP_HS_MASK

#define SIP_SSIP MIP_SSIP
#define SIP_STIP MIP_STIP

#define PRV_U 0
#define PRV_S 1
#define PRV_M 3

#define PRV_HS (PRV_S + 1)

#define SATP32_MODE 0x80000000
#define SATP32_ASID 0x7FC00000
#define SATP32_PPN  0x003FFFFF
#define SATP64_MODE 0xF000000000000000
#define SATP64_ASID 0x0FFFF00000000000
#define SATP64_PPN  0x00000FFFFFFFFFFF

#define SATP_MODE_OFF  0
#define SATP_MODE_SV32 1
#define SATP_MODE_SV39 8
#define SATP_MODE_SV48 9
#define SATP_MODE_SV57 10
#define SATP_MODE_SV64 11

#define HGATP32_MODE 0x80000000
#define HGATP32_VMID 0x1FC00000
#define HGATP32_PPN 0x003FFFFF

#define HGATP64_MODE 0xF000000000000000
#define HGATP64_VMID 0x03FFF00000000000
#define HGATP64_PPN 0x00000FFFFFFFFFFF

#define HGATP_MODE_OFF 0
#define HGATP_MODE_SV32X4 1
#define HGATP_MODE_SV39X4 8
#define HGATP_MODE_SV48X4 9

#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04
#define PMP_A     0x18
#define PMP_L     0x80
#define PMP_SHIFT 2

#define PMP_TOR   0x08
#define PMP_NA4   0x10
#define PMP_NAPOT 0x18

#define IRQ_U_SOFT   0
#define IRQ_S_SOFT   1
#define IRQ_VS_SOFT  2
#define IRQ_M_SOFT   3
#define IRQ_U_TIMER  4
#define IRQ_S_TIMER  5
#define IRQ_VS_TIMER 6
#define IRQ_M_TIMER  7
#define IRQ_U_EXT    8
#define IRQ_S_EXT    9
#define IRQ_VS_EXT   10
#define IRQ_M_EXT    11
#define IRQ_S_GEXT   12
#define IRQ_COP      12
#define IRQ_HOST     13

#define DEFAULT_RSTVEC     0xc0601000
#define CLINT_BASE         0xc0440000

#define CLINT_SIZE         0x000c0000
#define EXT_IO_BASE        0x40000000
#define DRAM_BASE          0x80000000
#define SYSDMA0_BASE       0xC9F00000
#define SYSDMA1_BASE       0xD1F00000
#define SYSDMA2_BASE       0xCFF00000
#define SYSDMA3_BASE       0xD7F00000
#define SYSDMA4_BASE       0xD9F00000
#define SYSDMA5_BASE       0xE1F00000
#define SYSDMA6_BASE       0xDFF00000
#define SYSDMA7_BASE       0xE7F00000

#define LLB_AXI0_BUFFER_START 0xD9000000
#define LLB_AXI1_BUFFER_START 0xE9000000
#define LLB_BUFFER_SIZE 0x2000000
#define LLB_BANK_BUFFER_SIZE  0x800000

#define GET_LLB_OFF(src, dst) do {                                                 \
    if(LLB_AXI0_BUFFER_START <= src < LLB_AXI0_BUFFER_START+LLB_BUFFER_SIZE)       \
        dst = src - LLB_AXI0_BUFFER_START;                                         \
    else if  (LLB_AXI1_BUFFER_START <= src < LLB_AXI1_BUFFER_START+LLB_BUFFER_SIZE)\
        dst = src - LLB_AXI1_BUFFER_START;                                         \
    else                                                                           \
        throw std::runtime_error("wrong llb address");                             \
} while(0)

/* page table entry (PTE) fields */
#define PTE_V     0x001 /* Valid */
#define PTE_R     0x002 /* Read */
#define PTE_W     0x004 /* Write */
#define PTE_X     0x008 /* Execute */
#define PTE_U     0x010 /* User */
#define PTE_G     0x020 /* Global */
#define PTE_A     0x040 /* Accessed */
#define PTE_D     0x080 /* Dirty */
#define PTE_SOFT  0x300 /* Reserved for Software */
#define PTE_N     0x4000000000000000 /* Zsn: NAPOT translation contiguity */

#define PTE_PPN_SHIFT 10

#define PTE_TABLE(PTE) (((PTE) & (PTE_V | PTE_R | PTE_W | PTE_X)) == PTE_V)

#ifdef __riscv

#if __riscv_xlen == 64
# define MSTATUS_SD MSTATUS64_SD
# define SSTATUS_SD SSTATUS64_SD
# define RISCV_PGLEVEL_BITS 9
# define SATP_MODE SATP64_MODE
#else
# define MSTATUS_SD MSTATUS32_SD
# define SSTATUS_SD SSTATUS32_SD
# define RISCV_PGLEVEL_BITS 10
# define SATP_MODE SATP32_MODE
#endif
#define RISCV_PGSHIFT 12
#define RISCV_PGSIZE (1 << RISCV_PGSHIFT)

#ifndef __ASSEMBLER__

#ifdef __GNUC__

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })

#define swap_csr(reg, val) ({ unsigned long __tmp; \
  asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "rK"(val)); \
  __tmp; })

#define set_csr(reg, bit) ({ unsigned long __tmp; \
  asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp; \
  asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })

#define rdtime() read_csr(time)
#define rdcycle() read_csr(cycle)
#define rdinstret() read_csr(instret)

#endif

#endif

#endif

#endif
/* Automatically generated by parse_opcodes.  */
#ifndef RISCV_ENCODING_H
#define RISCV_ENCODING_H
#define MATCH_SLLI_RV32 0x1013
#define MASK_SLLI_RV32  0xfe00707f
#define MATCH_SRLI_RV32 0x5013
#define MASK_SRLI_RV32  0xfe00707f
#define MATCH_SRAI_RV32 0x40005013
#define MASK_SRAI_RV32  0xfe00707f
#define MATCH_FRFLAGS 0x102073
#define MASK_FRFLAGS  0xfffff07f
#define MATCH_FSFLAGS 0x101073
#define MASK_FSFLAGS  0xfff0707f
#define MATCH_FSFLAGSI 0x105073
#define MASK_FSFLAGSI  0xfff0707f
#define MATCH_FRRM 0x202073
#define MASK_FRRM  0xfffff07f
#define MATCH_FSRM 0x201073
#define MASK_FSRM  0xfff0707f
#define MATCH_FSRMI 0x205073
#define MASK_FSRMI  0xfff0707f
#define MATCH_FSCSR 0x301073
#define MASK_FSCSR  0xfff0707f
#define MATCH_FRCSR 0x302073
#define MASK_FRCSR  0xfffff07f
#define MATCH_RDCYCLE 0xc0002073
#define MASK_RDCYCLE  0xfffff07f
#define MATCH_RDTIME 0xc0102073
#define MASK_RDTIME  0xfffff07f
#define MATCH_RDINSTRET 0xc0202073
#define MASK_RDINSTRET  0xfffff07f
#define MATCH_RDCYCLEH 0xc8002073
#define MASK_RDCYCLEH  0xfffff07f
#define MATCH_RDTIMEH 0xc8102073
#define MASK_RDTIMEH  0xfffff07f
#define MATCH_RDINSTRETH 0xc8202073
#define MASK_RDINSTRETH  0xfffff07f
#define MATCH_SCALL 0x73
#define MASK_SCALL  0xffffffff
#define MATCH_SBREAK 0x100073
#define MASK_SBREAK  0xffffffff
#define MATCH_FMV_X_S 0xe0000053
#define MASK_FMV_X_S  0xfff0707f
#define MATCH_FMV_S_X 0xf0000053
#define MASK_FMV_S_X  0xfff0707f
#define MATCH_FENCE_TSO 0x8330000f
#define MASK_FENCE_TSO  0xfff0707f
#define MATCH_PAUSE 0x100000f
#define MASK_PAUSE  0xffffffff
#define MATCH_BEQ 0x63
#define MASK_BEQ  0x707f
#define MATCH_BNE 0x1063
#define MASK_BNE  0x707f
#define MATCH_BLT 0x4063
#define MASK_BLT  0x707f
#define MATCH_BGE 0x5063
#define MASK_BGE  0x707f
#define MATCH_BLTU 0x6063
#define MASK_BLTU  0x707f
#define MATCH_BGEU 0x7063
#define MASK_BGEU  0x707f
#define MATCH_JALR 0x67
#define MASK_JALR  0x707f
#define MATCH_JAL 0x6f
#define MASK_JAL  0x7f
#define MATCH_LUI 0x37
#define MASK_LUI  0x7f
#define MATCH_AUIPC 0x17
#define MASK_AUIPC  0x7f
#define MATCH_ADDI 0x13
#define MASK_ADDI  0x707f
#define MATCH_SLLI 0x1013
#define MASK_SLLI  0xfc00707f
#define MATCH_SLTI 0x2013
#define MASK_SLTI  0x707f
#define MATCH_SLTIU 0x3013
#define MASK_SLTIU  0x707f
#define MATCH_XORI 0x4013
#define MASK_XORI  0x707f
#define MATCH_SRLI 0x5013
#define MASK_SRLI  0xfc00707f
#define MATCH_SRAI 0x40005013
#define MASK_SRAI  0xfc00707f
#define MATCH_ORI 0x6013
#define MASK_ORI  0x707f
#define MATCH_ANDI 0x7013
#define MASK_ANDI  0x707f
#define MATCH_ADD 0x33
#define MASK_ADD  0xfe00707f
#define MATCH_SUB 0x40000033
#define MASK_SUB  0xfe00707f
#define MATCH_SLL 0x1033
#define MASK_SLL  0xfe00707f
#define MATCH_SLT 0x2033
#define MASK_SLT  0xfe00707f
#define MATCH_SLTU 0x3033
#define MASK_SLTU  0xfe00707f
#define MATCH_XOR 0x4033
#define MASK_XOR  0xfe00707f
#define MATCH_SRL 0x5033
#define MASK_SRL  0xfe00707f
#define MATCH_SRA 0x40005033
#define MASK_SRA  0xfe00707f
#define MATCH_OR 0x6033
#define MASK_OR  0xfe00707f
#define MATCH_AND 0x7033
#define MASK_AND  0xfe00707f
#define MATCH_LB 0x3
#define MASK_LB  0x707f
#define MATCH_LH 0x1003
#define MASK_LH  0x707f
#define MATCH_LW 0x2003
#define MASK_LW  0x707f
#define MATCH_LBU 0x4003
#define MASK_LBU  0x707f
#define MATCH_LHU 0x5003
#define MASK_LHU  0x707f
#define MATCH_SB 0x23
#define MASK_SB  0x707f
#define MATCH_SH 0x1023
#define MASK_SH  0x707f
#define MATCH_SW 0x2023
#define MASK_SW  0x707f
#define MATCH_FENCE 0xf
#define MASK_FENCE  0x707f
#define MATCH_FENCE_I 0x100f
#define MASK_FENCE_I  0x707f
#define MATCH_ADDIW 0x1b
#define MASK_ADDIW  0x707f
#define MATCH_SLLIW 0x101b
#define MASK_SLLIW  0xfe00707f
#define MATCH_SRLIW 0x501b
#define MASK_SRLIW  0xfe00707f
#define MATCH_SRAIW 0x4000501b
#define MASK_SRAIW  0xfe00707f
#define MATCH_ADDW 0x3b
#define MASK_ADDW  0xfe00707f
#define MATCH_SUBW 0x4000003b
#define MASK_SUBW  0xfe00707f
#define MATCH_SLLW 0x103b
#define MASK_SLLW  0xfe00707f
#define MATCH_SRLW 0x503b
#define MASK_SRLW  0xfe00707f
#define MATCH_SRAW 0x4000503b
#define MASK_SRAW  0xfe00707f
#define MATCH_LD 0x3003
#define MASK_LD  0x707f
#define MATCH_LWU 0x6003
#define MASK_LWU  0x707f
#define MATCH_SD 0x3023
#define MASK_SD  0x707f
#define MATCH_MUL 0x2000033
#define MASK_MUL  0xfe00707f
#define MATCH_MULH 0x2001033
#define MASK_MULH  0xfe00707f
#define MATCH_MULHSU 0x2002033
#define MASK_MULHSU  0xfe00707f
#define MATCH_MULHU 0x2003033
#define MASK_MULHU  0xfe00707f
#define MATCH_DIV 0x2004033
#define MASK_DIV  0xfe00707f
#define MATCH_DIVU 0x2005033
#define MASK_DIVU  0xfe00707f
#define MATCH_REM 0x2006033
#define MASK_REM  0xfe00707f
#define MATCH_REMU 0x2007033
#define MASK_REMU  0xfe00707f
#define MATCH_MULW 0x200003b
#define MASK_MULW  0xfe00707f
#define MATCH_DIVW 0x200403b
#define MASK_DIVW  0xfe00707f
#define MATCH_DIVUW 0x200503b
#define MASK_DIVUW  0xfe00707f
#define MATCH_REMW 0x200603b
#define MASK_REMW  0xfe00707f
#define MATCH_REMUW 0x200703b
#define MASK_REMUW  0xfe00707f
#define MATCH_AMOADD_W 0x202f
#define MASK_AMOADD_W  0xf800707f
#define MATCH_AMOXOR_W 0x2000202f
#define MASK_AMOXOR_W  0xf800707f
#define MATCH_AMOOR_W 0x4000202f
#define MASK_AMOOR_W  0xf800707f
#define MATCH_AMOAND_W 0x6000202f
#define MASK_AMOAND_W  0xf800707f
#define MATCH_AMOMIN_W 0x8000202f
#define MASK_AMOMIN_W  0xf800707f
#define MATCH_AMOMAX_W 0xa000202f
#define MASK_AMOMAX_W  0xf800707f
#define MATCH_AMOMINU_W 0xc000202f
#define MASK_AMOMINU_W  0xf800707f
#define MATCH_AMOMAXU_W 0xe000202f
#define MASK_AMOMAXU_W  0xf800707f
#define MATCH_AMOSWAP_W 0x800202f
#define MASK_AMOSWAP_W  0xf800707f
#define MATCH_LR_W 0x1000202f
#define MASK_LR_W  0xf9f0707f
#define MATCH_SC_W 0x1800202f
#define MASK_SC_W  0xf800707f
#define MATCH_AMOADD_D 0x302f
#define MASK_AMOADD_D  0xf800707f
#define MATCH_AMOXOR_D 0x2000302f
#define MASK_AMOXOR_D  0xf800707f
#define MATCH_AMOOR_D 0x4000302f
#define MASK_AMOOR_D  0xf800707f
#define MATCH_AMOAND_D 0x6000302f
#define MASK_AMOAND_D  0xf800707f
#define MATCH_AMOMIN_D 0x8000302f
#define MASK_AMOMIN_D  0xf800707f
#define MATCH_AMOMAX_D 0xa000302f
#define MASK_AMOMAX_D  0xf800707f
#define MATCH_AMOMINU_D 0xc000302f
#define MASK_AMOMINU_D  0xf800707f
#define MATCH_AMOMAXU_D 0xe000302f
#define MASK_AMOMAXU_D  0xf800707f
#define MATCH_AMOSWAP_D 0x800302f
#define MASK_AMOSWAP_D  0xf800707f
#define MATCH_LR_D 0x1000302f
#define MASK_LR_D  0xf9f0707f
#define MATCH_SC_D 0x1800302f
#define MASK_SC_D  0xf800707f
#define MATCH_HFENCE_VVMA 0x22000073
#define MASK_HFENCE_VVMA  0xfe007fff
#define MATCH_HFENCE_GVMA 0x62000073
#define MASK_HFENCE_GVMA  0xfe007fff
#define MATCH_HLV_B 0x60004073
#define MASK_HLV_B  0xfff0707f
#define MATCH_HLV_BU 0x60104073
#define MASK_HLV_BU  0xfff0707f
#define MATCH_HLV_H 0x64004073
#define MASK_HLV_H  0xfff0707f
#define MATCH_HLV_HU 0x64104073
#define MASK_HLV_HU  0xfff0707f
#define MATCH_HLVX_HU 0x64304073
#define MASK_HLVX_HU  0xfff0707f
#define MATCH_HLV_W 0x68004073
#define MASK_HLV_W  0xfff0707f
#define MATCH_HLVX_WU 0x68304073
#define MASK_HLVX_WU  0xfff0707f
#define MATCH_HSV_B 0x62004073
#define MASK_HSV_B  0xfe007fff
#define MATCH_HSV_H 0x66004073
#define MASK_HSV_H  0xfe007fff
#define MATCH_HSV_W 0x6a004073
#define MASK_HSV_W  0xfe007fff
#define MATCH_HLV_WU 0x68104073
#define MASK_HLV_WU  0xfff0707f
#define MATCH_HLV_D 0x6c004073
#define MASK_HLV_D  0xfff0707f
#define MATCH_HSV_D 0x6e004073
#define MASK_HSV_D  0xfe007fff
#define MATCH_FADD_S 0x53
#define MASK_FADD_S  0xfe00007f
#define MATCH_FSUB_S 0x8000053
#define MASK_FSUB_S  0xfe00007f
#define MATCH_FMUL_S 0x10000053
#define MASK_FMUL_S  0xfe00007f
#define MATCH_FDIV_S 0x18000053
#define MASK_FDIV_S  0xfe00007f
#define MATCH_FSGNJ_S 0x20000053
#define MASK_FSGNJ_S  0xfe00707f
#define MATCH_FSGNJN_S 0x20001053
#define MASK_FSGNJN_S  0xfe00707f
#define MATCH_FSGNJX_S 0x20002053
#define MASK_FSGNJX_S  0xfe00707f
#define MATCH_FMIN_S 0x28000053
#define MASK_FMIN_S  0xfe00707f
#define MATCH_FMAX_S 0x28001053
#define MASK_FMAX_S  0xfe00707f
#define MATCH_FSQRT_S 0x58000053
#define MASK_FSQRT_S  0xfff0007f
#define MATCH_FLE_S 0xa0000053
#define MASK_FLE_S  0xfe00707f
#define MATCH_FLT_S 0xa0001053
#define MASK_FLT_S  0xfe00707f
#define MATCH_FEQ_S 0xa0002053
#define MASK_FEQ_S  0xfe00707f
#define MATCH_FCVT_W_S 0xc0000053
#define MASK_FCVT_W_S  0xfff0007f
#define MATCH_FCVT_WU_S 0xc0100053
#define MASK_FCVT_WU_S  0xfff0007f
#define MATCH_FMV_X_W 0xe0000053
#define MASK_FMV_X_W  0xfff0707f
#define MATCH_FCLASS_S 0xe0001053
#define MASK_FCLASS_S  0xfff0707f
#define MATCH_FCVT_S_W 0xd0000053
#define MASK_FCVT_S_W  0xfff0007f
#define MATCH_FCVT_S_WU 0xd0100053
#define MASK_FCVT_S_WU  0xfff0007f
#define MATCH_FMV_W_X 0xf0000053
#define MASK_FMV_W_X  0xfff0707f
#define MATCH_FLW 0x2007
#define MASK_FLW  0x707f
#define MATCH_FSW 0x2027
#define MASK_FSW  0x707f
#define MATCH_FMADD_S 0x43
#define MASK_FMADD_S  0x600007f
#define MATCH_FMSUB_S 0x47
#define MASK_FMSUB_S  0x600007f
#define MATCH_FNMSUB_S 0x4b
#define MASK_FNMSUB_S  0x600007f
#define MATCH_FNMADD_S 0x4f
#define MASK_FNMADD_S  0x600007f
#define MATCH_FCVT_L_S 0xc0200053
#define MASK_FCVT_L_S  0xfff0007f
#define MATCH_FCVT_LU_S 0xc0300053
#define MASK_FCVT_LU_S  0xfff0007f
#define MATCH_FCVT_S_L 0xd0200053
#define MASK_FCVT_S_L  0xfff0007f
#define MATCH_FCVT_S_LU 0xd0300053
#define MASK_FCVT_S_LU  0xfff0007f
#define MATCH_FADD_D 0x2000053
#define MASK_FADD_D  0xfe00007f
#define MATCH_FSUB_D 0xa000053
#define MASK_FSUB_D  0xfe00007f
#define MATCH_FMUL_D 0x12000053
#define MASK_FMUL_D  0xfe00007f
#define MATCH_FDIV_D 0x1a000053
#define MASK_FDIV_D  0xfe00007f
#define MATCH_FSGNJ_D 0x22000053
#define MASK_FSGNJ_D  0xfe00707f
#define MATCH_FSGNJN_D 0x22001053
#define MASK_FSGNJN_D  0xfe00707f
#define MATCH_FSGNJX_D 0x22002053
#define MASK_FSGNJX_D  0xfe00707f
#define MATCH_FMIN_D 0x2a000053
#define MASK_FMIN_D  0xfe00707f
#define MATCH_FMAX_D 0x2a001053
#define MASK_FMAX_D  0xfe00707f
#define MATCH_FCVT_S_D 0x40100053
#define MASK_FCVT_S_D  0xfff0007f
#define MATCH_FCVT_D_S 0x42000053
#define MASK_FCVT_D_S  0xfff0007f
#define MATCH_FSQRT_D 0x5a000053
#define MASK_FSQRT_D  0xfff0007f
#define MATCH_FLE_D 0xa2000053
#define MASK_FLE_D  0xfe00707f
#define MATCH_FLT_D 0xa2001053
#define MASK_FLT_D  0xfe00707f
#define MATCH_FEQ_D 0xa2002053
#define MASK_FEQ_D  0xfe00707f
#define MATCH_FCVT_W_D 0xc2000053
#define MASK_FCVT_W_D  0xfff0007f
#define MATCH_FCVT_WU_D 0xc2100053
#define MASK_FCVT_WU_D  0xfff0007f
#define MATCH_FCLASS_D 0xe2001053
#define MASK_FCLASS_D  0xfff0707f
#define MATCH_FCVT_D_W 0xd2000053
#define MASK_FCVT_D_W  0xfff0007f
#define MATCH_FCVT_D_WU 0xd2100053
#define MASK_FCVT_D_WU  0xfff0007f
#define MATCH_FLD 0x3007
#define MASK_FLD  0x707f
#define MATCH_FSD 0x3027
#define MASK_FSD  0x707f
#define MATCH_FMADD_D 0x2000043
#define MASK_FMADD_D  0x600007f
#define MATCH_FMSUB_D 0x2000047
#define MASK_FMSUB_D  0x600007f
#define MATCH_FNMSUB_D 0x200004b
#define MASK_FNMSUB_D  0x600007f
#define MATCH_FNMADD_D 0x200004f
#define MASK_FNMADD_D  0x600007f
#define MATCH_FCVT_L_D 0xc2200053
#define MASK_FCVT_L_D  0xfff0007f
#define MATCH_FCVT_LU_D 0xc2300053
#define MASK_FCVT_LU_D  0xfff0007f
#define MATCH_FMV_X_D 0xe2000053
#define MASK_FMV_X_D  0xfff0707f
#define MATCH_FCVT_D_L 0xd2200053
#define MASK_FCVT_D_L  0xfff0007f
#define MATCH_FCVT_D_LU 0xd2300053
#define MASK_FCVT_D_LU  0xfff0007f
#define MATCH_FMV_D_X 0xf2000053
#define MASK_FMV_D_X  0xfff0707f
#define MATCH_FADD_Q 0x6000053
#define MASK_FADD_Q  0xfe00007f
#define MATCH_FSUB_Q 0xe000053
#define MASK_FSUB_Q  0xfe00007f
#define MATCH_FMUL_Q 0x16000053
#define MASK_FMUL_Q  0xfe00007f
#define MATCH_FDIV_Q 0x1e000053
#define MASK_FDIV_Q  0xfe00007f
#define MATCH_FSGNJ_Q 0x26000053
#define MASK_FSGNJ_Q  0xfe00707f
#define MATCH_FSGNJN_Q 0x26001053
#define MASK_FSGNJN_Q  0xfe00707f
#define MATCH_FSGNJX_Q 0x26002053
#define MASK_FSGNJX_Q  0xfe00707f
#define MATCH_FMIN_Q 0x2e000053
#define MASK_FMIN_Q  0xfe00707f
#define MATCH_FMAX_Q 0x2e001053
#define MASK_FMAX_Q  0xfe00707f
#define MATCH_FCVT_S_Q 0x40300053
#define MASK_FCVT_S_Q  0xfff0007f
#define MATCH_FCVT_Q_S 0x46000053
#define MASK_FCVT_Q_S  0xfff0007f
#define MATCH_FCVT_D_Q 0x42300053
#define MASK_FCVT_D_Q  0xfff0007f
#define MATCH_FCVT_Q_D 0x46100053
#define MASK_FCVT_Q_D  0xfff0007f
#define MATCH_FSQRT_Q 0x5e000053
#define MASK_FSQRT_Q  0xfff0007f
#define MATCH_FLE_Q 0xa6000053
#define MASK_FLE_Q  0xfe00707f
#define MATCH_FLT_Q 0xa6001053
#define MASK_FLT_Q  0xfe00707f
#define MATCH_FEQ_Q 0xa6002053
#define MASK_FEQ_Q  0xfe00707f
#define MATCH_FCVT_W_Q 0xc6000053
#define MASK_FCVT_W_Q  0xfff0007f
#define MATCH_FCVT_WU_Q 0xc6100053
#define MASK_FCVT_WU_Q  0xfff0007f
#define MATCH_FCLASS_Q 0xe6001053
#define MASK_FCLASS_Q  0xfff0707f
#define MATCH_FCVT_Q_W 0xd6000053
#define MASK_FCVT_Q_W  0xfff0007f
#define MATCH_FCVT_Q_WU 0xd6100053
#define MASK_FCVT_Q_WU  0xfff0007f
#define MATCH_FLQ 0x4007
#define MASK_FLQ  0x707f
#define MATCH_FSQ 0x4027
#define MASK_FSQ  0x707f
#define MATCH_FMADD_Q 0x6000043
#define MASK_FMADD_Q  0x600007f
#define MATCH_FMSUB_Q 0x6000047
#define MASK_FMSUB_Q  0x600007f
#define MATCH_FNMSUB_Q 0x600004b
#define MASK_FNMSUB_Q  0x600007f
#define MATCH_FNMADD_Q 0x600004f
#define MASK_FNMADD_Q  0x600007f
#define MATCH_FCVT_L_Q 0xc6200053
#define MASK_FCVT_L_Q  0xfff0007f
#define MATCH_FCVT_LU_Q 0xc6300053
#define MASK_FCVT_LU_Q  0xfff0007f
#define MATCH_FCVT_Q_L 0xd6200053
#define MASK_FCVT_Q_L  0xfff0007f
#define MATCH_FCVT_Q_LU 0xd6300053
#define MASK_FCVT_Q_LU  0xfff0007f
#define MATCH_ANDN 0x40007033
#define MASK_ANDN  0xfe00707f
#define MATCH_ORN 0x40006033
#define MASK_ORN  0xfe00707f
#define MATCH_XNOR 0x40004033
#define MASK_XNOR  0xfe00707f
#define MATCH_SLO 0x20001033
#define MASK_SLO  0xfe00707f
#define MATCH_SRO 0x20005033
#define MASK_SRO  0xfe00707f
#define MATCH_ROL 0x60001033
#define MASK_ROL  0xfe00707f
#define MATCH_ROR 0x60005033
#define MASK_ROR  0xfe00707f
#define MATCH_SBCLR 0x48001033
#define MASK_SBCLR  0xfe00707f
#define MATCH_SBSET 0x28001033
#define MASK_SBSET  0xfe00707f
#define MATCH_SBINV 0x68001033
#define MASK_SBINV  0xfe00707f
#define MATCH_SBEXT 0x48005033
#define MASK_SBEXT  0xfe00707f
#define MATCH_GORC 0x28005033
#define MASK_GORC  0xfe00707f
#define MATCH_GREV 0x68005033
#define MASK_GREV  0xfe00707f
#define MATCH_SLOI 0x20001013
#define MASK_SLOI  0xfc00707f
#define MATCH_SROI 0x20005013
#define MASK_SROI  0xfc00707f
#define MATCH_RORI 0x60005013
#define MASK_RORI  0xfc00707f
#define MATCH_SBCLRI 0x48001013
#define MASK_SBCLRI  0xfc00707f
#define MATCH_SBSETI 0x28001013
#define MASK_SBSETI  0xfc00707f
#define MATCH_SBINVI 0x68001013
#define MASK_SBINVI  0xfc00707f
#define MATCH_SBEXTI 0x48005013
#define MASK_SBEXTI  0xfc00707f
#define MATCH_GORCI 0x28005013
#define MASK_GORCI  0xfc00707f
#define MATCH_GREVI 0x68005013
#define MASK_GREVI  0xfc00707f
#define MATCH_CMIX 0x6001033
#define MASK_CMIX  0x600707f
#define MATCH_CMOV 0x6005033
#define MASK_CMOV  0x600707f
#define MATCH_FSL 0x4001033
#define MASK_FSL  0x600707f
#define MATCH_FSR 0x4005033
#define MASK_FSR  0x600707f
#define MATCH_FSRI 0x4005013
#define MASK_FSRI  0x400707f
#define MATCH_CLZ 0x60001013
#define MASK_CLZ  0xfff0707f
#define MATCH_CTZ 0x60101013
#define MASK_CTZ  0xfff0707f
#define MATCH_PCNT 0x60201013
#define MASK_PCNT  0xfff0707f
#define MATCH_SEXT_B 0x60401013
#define MASK_SEXT_B  0xfff0707f
#define MATCH_SEXT_H 0x60501013
#define MASK_SEXT_H  0xfff0707f
#define MATCH_CRC32_B 0x61001013
#define MASK_CRC32_B  0xfff0707f
#define MATCH_CRC32_H 0x61101013
#define MASK_CRC32_H  0xfff0707f
#define MATCH_CRC32_W 0x61201013
#define MASK_CRC32_W  0xfff0707f
#define MATCH_CRC32C_B 0x61801013
#define MASK_CRC32C_B  0xfff0707f
#define MATCH_CRC32C_H 0x61901013
#define MASK_CRC32C_H  0xfff0707f
#define MATCH_CRC32C_W 0x61a01013
#define MASK_CRC32C_W  0xfff0707f
#define MATCH_SH1ADD 0x20002033
#define MASK_SH1ADD  0xfe00707f
#define MATCH_SH2ADD 0x20004033
#define MASK_SH2ADD  0xfe00707f
#define MATCH_SH3ADD 0x20006033
#define MASK_SH3ADD  0xfe00707f
#define MATCH_CLMUL 0xa001033
#define MASK_CLMUL  0xfe00707f
#define MATCH_CLMULR 0xa002033
#define MASK_CLMULR  0xfe00707f
#define MATCH_CLMULH 0xa003033
#define MASK_CLMULH  0xfe00707f
#define MATCH_MIN 0xa004033
#define MASK_MIN  0xfe00707f
#define MATCH_MINU 0xa005033
#define MASK_MINU  0xfe00707f
#define MATCH_MAX 0xa006033
#define MASK_MAX  0xfe00707f
#define MATCH_MAXU 0xa007033
#define MASK_MAXU  0xfe00707f
#define MATCH_SHFL 0x8001033
#define MASK_SHFL  0xfe00707f
#define MATCH_UNSHFL 0x8005033
#define MASK_UNSHFL  0xfe00707f
#define MATCH_BEXT 0x8006033
#define MASK_BEXT  0xfe00707f
#define MATCH_BDEP 0x48006033
#define MASK_BDEP  0xfe00707f
#define MATCH_PACK 0x8004033
#define MASK_PACK  0xfe00707f
#define MATCH_PACKU 0x48004033
#define MASK_PACKU  0xfe00707f
#define MATCH_PACKH 0x8007033
#define MASK_PACKH  0xfe00707f
#define MATCH_BFP 0x48007033
#define MASK_BFP  0xfe00707f
#define MATCH_SHFLI 0x8001013
#define MASK_SHFLI  0xfe00707f
#define MATCH_UNSHFLI 0x8005013
#define MASK_UNSHFLI  0xfe00707f
#define MATCH_BMATFLIP 0x60301013
#define MASK_BMATFLIP  0xfff0707f
#define MATCH_CRC32_D 0x61301013
#define MASK_CRC32_D  0xfff0707f
#define MATCH_CRC32C_D 0x61b01013
#define MASK_CRC32C_D  0xfff0707f
#define MATCH_BMATOR 0x8003033
#define MASK_BMATOR  0xfe00707f
#define MATCH_BMATXOR 0x48003033
#define MASK_BMATXOR  0xfe00707f
#define MATCH_ADDIWU 0x401b
#define MASK_ADDIWU  0x707f
#define MATCH_SLLIU_W 0x800101b
#define MASK_SLLIU_W  0xfc00707f
#define MATCH_ADDWU 0xa00003b
#define MASK_ADDWU  0xfe00707f
#define MATCH_SUBWU 0x4a00003b
#define MASK_SUBWU  0xfe00707f
#define MATCH_ADDU_W 0x800003b
#define MASK_ADDU_W  0xfe00707f
#define MATCH_SLOW 0x2000103b
#define MASK_SLOW  0xfe00707f
#define MATCH_SROW 0x2000503b
#define MASK_SROW  0xfe00707f
#define MATCH_ROLW 0x6000103b
#define MASK_ROLW  0xfe00707f
#define MATCH_RORW 0x6000503b
#define MASK_RORW  0xfe00707f
#define MATCH_SBCLRW 0x4800103b
#define MASK_SBCLRW  0xfe00707f
#define MATCH_SBSETW 0x2800103b
#define MASK_SBSETW  0xfe00707f
#define MATCH_SBINVW 0x6800103b
#define MASK_SBINVW  0xfe00707f
#define MATCH_SBEXTW 0x4800503b
#define MASK_SBEXTW  0xfe00707f
#define MATCH_GORCW 0x2800503b
#define MASK_GORCW  0xfe00707f
#define MATCH_GREVW 0x6800503b
#define MASK_GREVW  0xfe00707f
#define MATCH_SLOIW 0x2000101b
#define MASK_SLOIW  0xfe00707f
#define MATCH_SROIW 0x2000501b
#define MASK_SROIW  0xfe00707f
#define MATCH_RORIW 0x6000501b
#define MASK_RORIW  0xfe00707f
#define MATCH_SBCLRIW 0x4800101b
#define MASK_SBCLRIW  0xfe00707f
#define MATCH_SBSETIW 0x2800101b
#define MASK_SBSETIW  0xfe00707f
#define MATCH_SBINVIW 0x6800101b
#define MASK_SBINVIW  0xfe00707f
#define MATCH_GORCIW 0x2800501b
#define MASK_GORCIW  0xfe00707f
#define MATCH_GREVIW 0x6800501b
#define MASK_GREVIW  0xfe00707f
#define MATCH_FSLW 0x400103b
#define MASK_FSLW  0x600707f
#define MATCH_FSRW 0x400503b
#define MASK_FSRW  0x600707f
#define MATCH_FSRIW 0x400501b
#define MASK_FSRIW  0x600707f
#define MATCH_CLZW 0x6000101b
#define MASK_CLZW  0xfff0707f
#define MATCH_CTZW 0x6010101b
#define MASK_CTZW  0xfff0707f
#define MATCH_PCNTW 0x6020101b
#define MASK_PCNTW  0xfff0707f
#define MATCH_SH1ADDU_W 0x2000203b
#define MASK_SH1ADDU_W  0xfe00707f
#define MATCH_SH2ADDU_W 0x2000403b
#define MASK_SH2ADDU_W  0xfe00707f
#define MATCH_SH3ADDU_W 0x2000603b
#define MASK_SH3ADDU_W  0xfe00707f
#define MATCH_CLMULW 0xa00103b
#define MASK_CLMULW  0xfe00707f
#define MATCH_CLMULRW 0xa00203b
#define MASK_CLMULRW  0xfe00707f
#define MATCH_CLMULHW 0xa00303b
#define MASK_CLMULHW  0xfe00707f
#define MATCH_SHFLW 0x800103b
#define MASK_SHFLW  0xfe00707f
#define MATCH_UNSHFLW 0x800503b
#define MASK_UNSHFLW  0xfe00707f
#define MATCH_BEXTW 0x800603b
#define MASK_BEXTW  0xfe00707f
#define MATCH_BDEPW 0x4800603b
#define MASK_BDEPW  0xfe00707f
#define MATCH_PACKW 0x800403b
#define MASK_PACKW  0xfe00707f
#define MATCH_PACKUW 0x4800403b
#define MASK_PACKUW  0xfe00707f
#define MATCH_BFPW 0x4800703b
#define MASK_BFPW  0xfe00707f
#define MATCH_FADD_H 0x4000053
#define MASK_FADD_H  0xfe00007f
#define MATCH_FSUB_H 0xc000053
#define MASK_FSUB_H  0xfe00007f
#define MATCH_FMUL_H 0x14000053
#define MASK_FMUL_H  0xfe00007f
#define MATCH_FDIV_H 0x1c000053
#define MASK_FDIV_H  0xfe00007f
#define MATCH_FSGNJ_H 0x24000053
#define MASK_FSGNJ_H  0xfe00707f
#define MATCH_FSGNJN_H 0x24001053
#define MASK_FSGNJN_H  0xfe00707f
#define MATCH_FSGNJX_H 0x24002053
#define MASK_FSGNJX_H  0xfe00707f
#define MATCH_FMIN_H 0x2c000053
#define MASK_FMIN_H  0xfe00707f
#define MATCH_FMAX_H 0x2c001053
#define MASK_FMAX_H  0xfe00707f
#define MATCH_FCVT_H_S 0x44000053
#define MASK_FCVT_H_S  0xfff0007f
#define MATCH_FCVT_S_H 0x40200053
#define MASK_FCVT_S_H  0xfff0007f
#define MATCH_FSQRT_H 0x5c000053
#define MASK_FSQRT_H  0xfff0007f
#define MATCH_FLE_H 0xa4000053
#define MASK_FLE_H  0xfe00707f
#define MATCH_FLT_H 0xa4001053
#define MASK_FLT_H  0xfe00707f
#define MATCH_FEQ_H 0xa4002053
#define MASK_FEQ_H  0xfe00707f
#define MATCH_FCVT_W_H 0xc4000053
#define MASK_FCVT_W_H  0xfff0007f
#define MATCH_FCVT_WU_H 0xc4100053
#define MASK_FCVT_WU_H  0xfff0007f
#define MATCH_FMV_X_H 0xe4000053
#define MASK_FMV_X_H  0xfff0707f
#define MATCH_FCLASS_H 0xe4001053
#define MASK_FCLASS_H  0xfff0707f
#define MATCH_FCVT_H_W 0xd4000053
#define MASK_FCVT_H_W  0xfff0007f
#define MATCH_FCVT_H_WU 0xd4100053
#define MASK_FCVT_H_WU  0xfff0007f
#define MATCH_FMV_H_X 0xf4000053
#define MASK_FMV_H_X  0xfff0707f
#define MATCH_FLH 0x1007
#define MASK_FLH  0x707f
#define MATCH_FSH 0x1027
#define MASK_FSH  0x707f
#define MATCH_FMADD_H 0x4000043
#define MASK_FMADD_H  0x600007f
#define MATCH_FMSUB_H 0x4000047
#define MASK_FMSUB_H  0x600007f
#define MATCH_FNMSUB_H 0x400004b
#define MASK_FNMSUB_H  0x600007f
#define MATCH_FNMADD_H 0x400004f
#define MASK_FNMADD_H  0x600007f
#define MATCH_FCVT_H_Q 0x44300053
#define MASK_FCVT_H_Q  0xfff0007f
#define MATCH_FCVT_Q_H 0x46200053
#define MASK_FCVT_Q_H  0xfff0007f
#define MATCH_FCVT_L_H 0xc4200053
#define MASK_FCVT_L_H  0xfff0007f
#define MATCH_FCVT_LU_H 0xc4300053
#define MASK_FCVT_LU_H  0xfff0007f
#define MATCH_FCVT_H_L 0xd4200053
#define MASK_FCVT_H_L  0xfff0007f
#define MATCH_FCVT_H_LU 0xd4300053
#define MASK_FCVT_H_LU  0xfff0007f
#define MATCH_FCVT_H_D 0x44100053
#define MASK_FCVT_H_D  0xfff0007f
#define MATCH_FCVT_D_H 0x42200053
#define MASK_FCVT_D_H  0xfff0007f
#define MATCH_FMV_X_Q 0xe6000053
#define MASK_FMV_X_Q  0xfff0707f
#define MATCH_FMV_Q_X 0xf6000053
#define MASK_FMV_Q_X  0xfff0707f
#define MATCH_ECALL 0x73
#define MASK_ECALL  0xffffffff
#define MATCH_EBREAK 0x100073
#define MASK_EBREAK  0xffffffff
#define MATCH_URET 0x200073
#define MASK_URET  0xffffffff
#define MATCH_SRET 0x10200073
#define MASK_SRET  0xffffffff
#define MATCH_MRET 0x30200073
#define MASK_MRET  0xffffffff
#define MATCH_DRET 0x7b200073
#define MASK_DRET  0xffffffff
#define MATCH_SFENCE_VMA 0x12000073
#define MASK_SFENCE_VMA  0xfe007fff
#define MATCH_WFI 0x10500073
#define MASK_WFI  0xffffffff
#define MATCH_CSRRW 0x1073
#define MASK_CSRRW  0x707f
#define MATCH_CSRRS 0x2073
#define MASK_CSRRS  0x707f
#define MATCH_CSRRC 0x3073
#define MASK_CSRRC  0x707f
#define MATCH_CSRRWI 0x5073
#define MASK_CSRRWI  0x707f
#define MATCH_CSRRSI 0x6073
#define MASK_CSRRSI  0x707f
#define MATCH_CSRRCI 0x7073
#define MASK_CSRRCI  0x707f
#define MATCH_C_NOP 0x1
#define MASK_C_NOP  0xffff
#define MATCH_C_ADDI16SP 0x6101
#define MASK_C_ADDI16SP  0xef83
#define MATCH_C_JR 0x8002
#define MASK_C_JR  0xf07f
#define MATCH_C_JALR 0x9002
#define MASK_C_JALR  0xf07f
#define MATCH_C_EBREAK 0x9002
#define MASK_C_EBREAK  0xffff
#define MATCH_C_ADDI4SPN 0x0
#define MASK_C_ADDI4SPN  0xe003
#define MATCH_C_FLD 0x2000
#define MASK_C_FLD  0xe003
#define MATCH_C_LW 0x4000
#define MASK_C_LW  0xe003
#define MATCH_C_FLW 0x6000
#define MASK_C_FLW  0xe003
#define MATCH_C_FSD 0xa000
#define MASK_C_FSD  0xe003
#define MATCH_C_SW 0xc000
#define MASK_C_SW  0xe003
#define MATCH_C_FSW 0xe000
#define MASK_C_FSW  0xe003
#define MATCH_C_ADDI 0x1
#define MASK_C_ADDI  0xe003
#define MATCH_C_JAL 0x2001
#define MASK_C_JAL  0xe003
#define MATCH_C_LI 0x4001
#define MASK_C_LI  0xe003
#define MATCH_C_LUI 0x6001
#define MASK_C_LUI  0xe003
#define MATCH_C_SRLI 0x8001
#define MASK_C_SRLI  0xec03
#define MATCH_C_SRAI 0x8401
#define MASK_C_SRAI  0xec03
#define MATCH_C_ANDI 0x8801
#define MASK_C_ANDI  0xec03
#define MATCH_C_SUB 0x8c01
#define MASK_C_SUB  0xfc63
#define MATCH_C_XOR 0x8c21
#define MASK_C_XOR  0xfc63
#define MATCH_C_OR 0x8c41
#define MASK_C_OR  0xfc63
#define MATCH_C_AND 0x8c61
#define MASK_C_AND  0xfc63
#define MATCH_C_J 0xa001
#define MASK_C_J  0xe003
#define MATCH_C_BEQZ 0xc001
#define MASK_C_BEQZ  0xe003
#define MATCH_C_BNEZ 0xe001
#define MASK_C_BNEZ  0xe003
#define MATCH_C_SLLI 0x2
#define MASK_C_SLLI  0xe003
#define MATCH_C_FLDSP 0x2002
#define MASK_C_FLDSP  0xe003
#define MATCH_C_LWSP 0x4002
#define MASK_C_LWSP  0xe003
#define MATCH_C_FLWSP 0x6002
#define MASK_C_FLWSP  0xe003
#define MATCH_C_MV 0x8002
#define MASK_C_MV  0xf003
#define MATCH_C_ADD 0x9002
#define MASK_C_ADD  0xf003
#define MATCH_C_FSDSP 0xa002
#define MASK_C_FSDSP  0xe003
#define MATCH_C_SWSP 0xc002
#define MASK_C_SWSP  0xe003
#define MATCH_C_FSWSP 0xe002
#define MASK_C_FSWSP  0xe003
#define MATCH_C_SRLI_RV32 0x8001
#define MASK_C_SRLI_RV32  0xfc03
#define MATCH_C_SRAI_RV32 0x8401
#define MASK_C_SRAI_RV32  0xfc03
#define MATCH_C_SLLI_RV32 0x2
#define MASK_C_SLLI_RV32  0xf003
#define MATCH_C_LD 0x6000
#define MASK_C_LD  0xe003
#define MATCH_C_SD 0xe000
#define MASK_C_SD  0xe003
#define MATCH_C_SUBW 0x9c01
#define MASK_C_SUBW  0xfc63
#define MATCH_C_ADDW 0x9c21
#define MASK_C_ADDW  0xfc63
#define MATCH_C_ADDIW 0x2001
#define MASK_C_ADDIW  0xe003
#define MATCH_C_LDSP 0x6002
#define MASK_C_LDSP  0xe003
#define MATCH_C_SDSP 0xe002
#define MASK_C_SDSP  0xe003
#define MATCH_CUSTOM0 0xb
#define MASK_CUSTOM0  0x707f
#define MATCH_CUSTOM0_RS1 0x200b
#define MASK_CUSTOM0_RS1  0x707f
#define MATCH_CUSTOM0_RS1_RS2 0x300b
#define MASK_CUSTOM0_RS1_RS2  0x707f
#define MATCH_CUSTOM0_RD 0x400b
#define MASK_CUSTOM0_RD  0x707f
#define MATCH_CUSTOM0_RD_RS1 0x600b
#define MASK_CUSTOM0_RD_RS1  0x707f
#define MATCH_CUSTOM0_RD_RS1_RS2 0x700b
#define MASK_CUSTOM0_RD_RS1_RS2  0x707f
#define MATCH_CUSTOM1 0x2b
#define MASK_CUSTOM1  0x707f
#define MATCH_CUSTOM1_RS1 0x202b
#define MASK_CUSTOM1_RS1  0x707f
#define MATCH_CUSTOM1_RS1_RS2 0x302b
#define MASK_CUSTOM1_RS1_RS2  0x707f
#define MATCH_CUSTOM1_RD 0x402b
#define MASK_CUSTOM1_RD  0x707f
#define MATCH_CUSTOM1_RD_RS1 0x602b
#define MASK_CUSTOM1_RD_RS1  0x707f
#define MATCH_CUSTOM1_RD_RS1_RS2 0x702b
#define MASK_CUSTOM1_RD_RS1_RS2  0x707f
#define MATCH_CUSTOM2 0x5b
#define MASK_CUSTOM2  0x707f
#define MATCH_CUSTOM2_RS1 0x205b
#define MASK_CUSTOM2_RS1  0x707f
#define MATCH_CUSTOM2_RS1_RS2 0x305b
#define MASK_CUSTOM2_RS1_RS2  0x707f
#define MATCH_CUSTOM2_RD 0x405b
#define MASK_CUSTOM2_RD  0x707f
#define MATCH_CUSTOM2_RD_RS1 0x605b
#define MASK_CUSTOM2_RD_RS1  0x707f
#define MATCH_CUSTOM2_RD_RS1_RS2 0x705b
#define MASK_CUSTOM2_RD_RS1_RS2  0x707f
#define MATCH_CUSTOM3 0x7b
#define MASK_CUSTOM3  0x707f
#define MATCH_CUSTOM3_RS1 0x207b
#define MASK_CUSTOM3_RS1  0x707f
#define MATCH_CUSTOM3_RS1_RS2 0x307b
#define MASK_CUSTOM3_RS1_RS2  0x707f
#define MATCH_CUSTOM3_RD 0x407b
#define MASK_CUSTOM3_RD  0x707f
#define MATCH_CUSTOM3_RD_RS1 0x607b
#define MASK_CUSTOM3_RD_RS1  0x707f
#define MATCH_CUSTOM3_RD_RS1_RS2 0x707b
#define MASK_CUSTOM3_RD_RS1_RS2  0x707f
#define MATCH_VECVT_HF_X8_M 0x3000107b
#define MASK_VECVT_HF_X8_M  0xfff0707f
#define MATCH_VECVT_HF_XU8_M 0x3010107b
#define MASK_VECVT_HF_XU8_M  0xfff0707f
#define MATCH_VECVT_X8_HF_M 0x3020107b
#define MASK_VECVT_X8_HF_M  0xfff0707f
#define MATCH_VECVT_XU8_HF_M 0x3030107b
#define MASK_VECVT_XU8_HF_M  0xfff0707f
#define MATCH_VECVT_HF_X16_M 0x3040107b
#define MASK_VECVT_HF_X16_M  0xfff0707f
#define MATCH_VECVT_X16_HF_M 0x3050107b
#define MASK_VECVT_X16_HF_M  0xfff0707f
#define MATCH_VECVT_F32_HF_M 0x3060107b
#define MASK_VECVT_F32_HF_M  0xfff0707f
#define MATCH_VECVT_HF_F32_M 0x3070107b
#define MASK_VECVT_HF_F32_M  0xfff0707f
#define MATCH_VECVT_BF_X8_M 0x3080107b
#define MASK_VECVT_BF_X8_M  0xfff0707f
#define MATCH_VECVT_BF_XU8_M 0x3090107b
#define MASK_VECVT_BF_XU8_M  0xfff0707f
#define MATCH_VECVT_X8_BF_M 0x30a0107b
#define MASK_VECVT_X8_BF_M  0xfff0707f
#define MATCH_VECVT_XU8_BF_M 0x30b0107b
#define MASK_VECVT_XU8_BF_M  0xfff0707f
#define MATCH_VECVT_BF_X16_M 0x30c0107b
#define MASK_VECVT_BF_X16_M  0xfff0707f
#define MATCH_VECVT_X16_BF_M 0x30d0107b
#define MASK_VECVT_X16_BF_M  0xfff0707f
#define MATCH_VECVT_F32_BF_M 0x30e0107b
#define MASK_VECVT_F32_BF_M  0xfff0707f
#define MATCH_VECVT_BF_F32_M 0x30f0107b
#define MASK_VECVT_BF_F32_M  0xfff0707f
#define MATCH_VECVT_BF_HF_M 0x3100107b
#define MASK_VECVT_BF_HF_M  0xfff0707f
#define MATCH_VECVT_HF_BF_M 0x3110107b
#define MASK_VECVT_HF_BF_M  0xfff0707f
#define MATCH_VECVT_F32_X32_M 0x3120107b
#define MASK_VECVT_F32_X32_M  0xfff0707f
#define MATCH_VECVT_X32_F32_M 0x3130107b
#define MASK_VECVT_X32_F32_M  0xfff0707f
#define MATCH_VEADD_MM 0x400007b
#define MASK_VEADD_MM  0xfe00707f
#define MATCH_VEADD_MV 0x600207b
#define MASK_VEADD_MV  0xfe00307f
#define MATCH_VEADD_MF 0x400307b
#define MASK_VEADD_MF  0xfe00707f
#define MATCH_VEADD_RELU_MM 0x400407b
#define MASK_VEADD_RELU_MM  0xfe00707f
#define MATCH_VEADD_RELU_MV 0x400207b
#define MASK_VEADD_RELU_MV  0xfe00307f
#define MATCH_VEADD_RELU_MF 0x400707b
#define MASK_VEADD_RELU_MF  0xfe00707f
#define MATCH_VESUB_MM 0x800007b
#define MASK_VESUB_MM  0xfe00707f
#define MATCH_VESUB_MV 0xa00207b
#define MASK_VESUB_MV  0xfe00307f
#define MATCH_VESUB_MF 0x800307b
#define MASK_VESUB_MF  0xfe00707f
#define MATCH_VEEMUL_MM 0x1400007b
#define MASK_VEEMUL_MM  0xfe00707f
#define MATCH_VEEMUL_MV 0x1600207b
#define MASK_VEEMUL_MV  0xfe00307f
#define MATCH_VEEMUL_MF 0x1400307b
#define MASK_VEEMUL_MF  0xfe00707f
#define MATCH_VEEMUL_RELU_MV 0x1400207b
#define MASK_VEEMUL_RELU_MV  0xfe00307f
#define MATCH_VEEMUL_X8_HF_MF 0x1000307b
#define MASK_VEEMUL_X8_HF_MF  0xfe00707f
#define MATCH_VEEMUL_XU8_HF_MF 0x1000707b
#define MASK_VEEMUL_XU8_HF_MF  0xfe00707f
#define MATCH_VEEMUL_X8_BF_MF 0x1200307b
#define MASK_VEEMUL_X8_BF_MF  0xfe00707f
#define MATCH_VEEMUL_XU8_BF_MF 0x1200707b
#define MASK_VEEMUL_XU8_BF_MF  0xfe00707f
#define MATCH_VEEMUL_RELU_MM 0x1400407b
#define MASK_VEEMUL_RELU_MM  0xfe00707f
#define MATCH_VEEMUL_RELU_MF 0x1400707b
#define MASK_VEEMUL_RELU_MF  0xfe00707f
#define MATCH_VEEMACC_MM 0x1800007b
#define MASK_VEEMACC_MM  0xfc00307f
#define MATCH_VEEMACC_MV 0x1a00207b
#define MASK_VEEMACC_MV  0xfe00307f
#define MATCH_VEEMACC_MF 0x1a00307b
#define MASK_VEEMACC_MF  0xfe00307f
#define MATCH_VEACC_M 0xc00107b
#define MASK_VEACC_M  0xfdf0307f
#define MATCH_VEMAX_M 0x1c00107b
#define MASK_VEMAX_M  0xfdf0307f
#define MATCH_VEMIN_M 0x2000107b
#define MASK_VEMIN_M  0xfdf0307f
#define MATCH_VEARGMAX_M 0x1c10107b
#define MASK_VEARGMAX_M  0xfdf0307f
#define MATCH_VEARGMIN_M 0x2010107b
#define MASK_VEARGMIN_M  0xfdf0307f
#define MATCH_VEMAX_MM 0x1c00007b
#define MASK_VEMAX_MM  0xfe00707f
#define MATCH_VEMAX_MV 0x1e00207b
#define MASK_VEMAX_MV  0xfe00307f
#define MATCH_VEMAX_MF 0x1c00307b
#define MASK_VEMAX_MF  0xfe00707f
#define MATCH_VEMIN_MM 0x2000007b
#define MASK_VEMIN_MM  0xfe00707f
#define MATCH_VEMIN_MV 0x2200207b
#define MASK_VEMIN_MV  0xfe00307f
#define MATCH_VEMIN_MF 0x2000307b
#define MASK_VEMIN_MF  0xfe00707f
#define MATCH_VELKRELU_MV 0x2600207b
#define MASK_VELKRELU_MV  0xfe00307f
#define MATCH_VELKRELU_MF 0x2400307b
#define MASK_VELKRELU_MF  0xfe00707f
#define MATCH_VELUT_M 0x2800107b
#define MASK_VELUT_M  0xfe00707f
#define MATCH_MOV_M 0x2c00107b
#define MASK_MOV_M  0xfff0707f
#define MATCH_MOV_LLB_L1 0x2c10107b
#define MASK_MOV_LLB_L1  0xfff0707f
#define MATCH_MOV_L1_LLB 0x2c20107b
#define MASK_MOV_L1_LLB  0xfff0707f
#define MATCH_MOV_L1_GLB 0x2c30107b
#define MASK_MOV_L1_GLB  0xfff0707f
#define MATCH_MOV_LLB_GLB 0x2c40107b
#define MASK_MOV_LLB_GLB  0xfff0707f
#define MATCH_MOV_GLB_LLB 0x2c50107b
#define MASK_MOV_GLB_LLB  0xfff0707f
#define MATCH_MOV_GLB_L1 0x2c60107b
#define MASK_MOV_GLB_L1  0xfff0707f
#define MATCH_MOV_GLB_GLB 0x2c70107b
#define MASK_MOV_GLB_GLB  0xfff0707f
#define MATCH_MOV_LLB_LLB 0x2c80107b
#define MASK_MOV_LLB_LLB  0xfff0707f
#define MATCH_MOV_V 0x2e00207b
#define MASK_MOV_V  0xfff0307f
#define MATCH_MOV_F 0x2c00307b
#define MASK_MOV_F  0xfff0707f
#define MATCH_ICMOV_M 0x3400107b
#define MASK_ICMOV_M  0xfff0707f
#define MATCH_SYNC 0x3800007b
#define MASK_SYNC  0xffffffff
#define MATCH_SYNC_MTE 0x3800407b
#define MASK_SYNC_MTE  0xffffffff
#define MATCH_SYNC_DMAE 0x3a00007b
#define MASK_SYNC_DMAE  0xffffffff
#define MATCH_SYNC_MTE_R 0x3800107b
#define MASK_SYNC_MTE_R  0xfff07fff
#define MATCH_SYNC_MTE_W 0x3800507b
#define MASK_SYNC_MTE_W  0xfffff07f
#define MATCH_SYNC_DMAE_R 0x3a00107b
#define MASK_SYNC_DMAE_R  0xfff07fff
#define MATCH_SYNC_DMAE_W 0x3a00507b
#define MASK_SYNC_DMAE_W  0xfffff07f
#define MATCH_FLHW 0x105b
#define MASK_FLHW  0x707f
#define MATCH_FSHW 0x405b
#define MASK_FSHW  0x707f
#define MATCH_METR_M 0x6000107b
#define MASK_METR_M  0xfff0707f
#define MATCH_MECONV_MM 0x6800007b
#define MASK_MECONV_MM  0xfe00707f
#define MATCH_MECONV_SP_MM 0x6800407b
#define MASK_MECONV_SP_MM  0xfe00707f
#define MATCH_MEDECONV_MM 0x6a00007b
#define MASK_MEDECONV_MM  0xfe00707f
#define MATCH_MEDECONV_SP_MM 0x6a00407b
#define MASK_MEDECONV_SP_MM  0xfe00707f
#define MATCH_PLD 0x3c00107b
#define MASK_PLD  0xfe00707f
#define MATCH_VEEXP_M 0x4000107b
#define MASK_VEEXP_M  0xfff0707f
#define MATCH_VESQRT_M 0x4010107b
#define MASK_VESQRT_M  0xfff0707f
#define MATCH_VERECIP_M 0x4020107b
#define MASK_VERECIP_M  0xfff0707f
#define MATCH_VERSQRT_M 0x4030107b
#define MASK_VERSQRT_M  0xfff0707f
#define MATCH_VELN_M 0x4040107b
#define MASK_VELN_M  0xfff0707f
#define MATCH_VESIN_M 0x4050107b
#define MASK_VESIN_M  0xfff0707f
#define MATCH_VECOS_M 0x4060107b
#define MASK_VECOS_M  0xfff0707f
#define MATCH_VETANH_M 0x4070107b
#define MASK_VETANH_M  0xfff0707f
#define MATCH_VESIGMOID_M 0x4080107b
#define MASK_VESIGMOID_M  0xfff0707f
#define MATCH_VESINH_M 0x4090107b
#define MASK_VESINH_M  0xfff0707f
#define MATCH_VECOSH_M 0x40a0107b
#define MASK_VECOSH_M  0xfff0707f
#define MATCH_VEDIV_MM 0x4400007b
#define MASK_VEDIV_MM  0xfe00707f
#define MATCH_VEDIV_MV 0x4600207b
#define MASK_VEDIV_MV  0xfe00307f
#define MATCH_VEDIV_MF 0x4400307b
#define MASK_VEDIV_MF  0xfe00707f
#define MATCH_VEAVGPOOL_M 0x4800107b
#define MASK_VEAVGPOOL_M  0xfff0707f
#define MATCH_VEMAXPOOL_M 0x4800507b
#define MASK_VEMAXPOOL_M  0xfff0707f
#define MATCH_VEDWCONV_MM 0x4800007b
#define MASK_VEDWCONV_MM  0xfe00707f
#define MATCH_VERSUB_MV 0x4e00207b
#define MASK_VERSUB_MV  0xfe00307f
#define MATCH_VERSUB_MF 0x4c00307b
#define MASK_VERSUB_MF  0xfe00707f
#define MATCH_VEROT180_M 0x5000107b
#define MASK_VEROT180_M  0xfff0707f
#define MATCH_VEREV_M 0x5200107b
#define MASK_VEREV_M  0xfff0307f
#define MATCH_VERAND_V 0x5400107b
#define MASK_VERAND_V  0xfff0707f
#define MATCH_VERAND_M 0x5400507b
#define MASK_VERAND_M  0xfffff07f
#define MATCH_MEMUL_MM 0x6400007b
#define MASK_MEMUL_MM  0xfe00707f
#define MATCH_MEMUL_SP_MM 0x6400407b
#define MASK_MEMUL_SP_MM  0xfe00707f
#define MATCH_MEMUL_TS1_MM 0x6c00007b
#define MASK_MEMUL_TS1_MM  0xfe00707f
#define MATCH_MEMIN_M 0x7000107b
#define MASK_MEMIN_M  0xfff0707f
#define MATCH_MEMAX_M 0x7000507b
#define MASK_MEMAX_M  0xfff0707f
#define MATCH_MEACC_M 0x7200107b
#define MASK_MEACC_M  0xfff0707f
#define MATCH_MEDWCONV_MM 0x7400007b
#define MASK_MEDWCONV_MM  0xfe00707f
#define MATCH_VSETVLI 0x7057
#define MASK_VSETVLI  0x8000707f
#define MATCH_VSETVL 0x80007057
#define MASK_VSETVL  0xfe00707f
#define MATCH_VLE8_V 0x7
#define MASK_VLE8_V  0x1df0707f
#define MATCH_VLE8PI_V 0x01100007
#define MASK_VLE8PI_V  0x1df0707f
#define MATCH_VLE16_V 0x5007
#define MASK_VLE16_V  0x1df0707f
#define MATCH_VLE16PI_V 0x01105007
#define MASK_VLE16PI_V  0x1df0707f
#define MATCH_VLE32_V 0x6007
#define MASK_VLE32_V  0x1df0707f
#define MATCH_VLE32PI_V 0x01106007
#define MASK_VLE32PI_V  0x1df0707f
#define MATCH_VLE64_V 0x7007
#define MASK_VLE64_V  0x1df0707f
#define MATCH_VLE128_V 0x10000007
#define MASK_VLE128_V  0x1df0707f
#define MATCH_VLE256_V 0x10005007
#define MASK_VLE256_V  0x1df0707f
#define MATCH_VLE512_V 0x10006007
#define MASK_VLE512_V  0x1df0707f
#define MATCH_VLE1024_V 0x10007007
#define MASK_VLE1024_V  0x1df0707f
#define MATCH_VSE8_V 0x27
#define MASK_VSE8_V  0x1df0707f
#define MATCH_VSE8PI_V 0x01100027
#define MASK_VSE8PI_V  0x1df0707f
#define MATCH_VSE16_V 0x5027
#define MASK_VSE16_V  0x1df0707f
#define MATCH_VSE16PI_V 0x01105027
#define MASK_VSE16PI_V  0x1df0707f
#define MATCH_VSE32_V 0x6027
#define MASK_VSE32_V  0x1df0707f
#define MATCH_VSE32PI_V 0x01106027
#define MASK_VSE32PI_V  0x1df0707f
#define MATCH_VSE64_V 0x7027
#define MASK_VSE64_V  0x1df0707f
#define MATCH_VSE128_V 0x10000027
#define MASK_VSE128_V  0x1df0707f
#define MATCH_VSE256_V 0x10005027
#define MASK_VSE256_V  0x1df0707f
#define MATCH_VSE512_V 0x10006027
#define MASK_VSE512_V  0x1df0707f
#define MATCH_VSE1024_V 0x10007027
#define MASK_VSE1024_V  0x1df0707f
#define MATCH_VLUXEI8_V 0x4000007
#define MASK_VLUXEI8_V  0x1c00707f
#define MATCH_VLUXEI16_V 0x4005007
#define MASK_VLUXEI16_V  0x1c00707f
#define MATCH_VLUXEI32_V 0x4006007
#define MASK_VLUXEI32_V  0x1c00707f
#define MATCH_VLUXEI64_V 0x4007007
#define MASK_VLUXEI64_V  0x1c00707f
#define MATCH_VLUXEI128_V 0x14000007
#define MASK_VLUXEI128_V  0x1c00707f
#define MATCH_VLUXEI256_V 0x14005007
#define MASK_VLUXEI256_V  0x1c00707f
#define MATCH_VLUXEI512_V 0x14006007
#define MASK_VLUXEI512_V  0x1c00707f
#define MATCH_VLUXEI1024_V 0x14007007
#define MASK_VLUXEI1024_V  0x1c00707f
#define MATCH_VSUXEI8_V 0x4000027
#define MASK_VSUXEI8_V  0x1c00707f
#define MATCH_VSUXEI16_V 0x4005027
#define MASK_VSUXEI16_V  0x1c00707f
#define MATCH_VSUXEI32_V 0x4006027
#define MASK_VSUXEI32_V  0x1c00707f
#define MATCH_VSUXEI64_V 0x4007027
#define MASK_VSUXEI64_V  0x1c00707f
#define MATCH_VSUXEI128_V 0x14000027
#define MASK_VSUXEI128_V  0x1c00707f
#define MATCH_VSUXEI256_V 0x14005027
#define MASK_VSUXEI256_V  0x1c00707f
#define MATCH_VSUXEI512_V 0x14006027
#define MASK_VSUXEI512_V  0x1c00707f
#define MATCH_VSUXEI1024_V 0x14007027
#define MASK_VSUXEI1024_V  0x1c00707f
#define MATCH_VLSE8_V 0x8000007
#define MASK_VLSE8_V  0x1c00707f
#define MATCH_VLSE16_V 0x8005007
#define MASK_VLSE16_V  0x1c00707f
#define MATCH_VLSE32_V 0x8006007
#define MASK_VLSE32_V  0x1c00707f
#define MATCH_VLSE64_V 0x8007007
#define MASK_VLSE64_V  0x1c00707f
#define MATCH_VLSE128_V 0x18000007
#define MASK_VLSE128_V  0x1c00707f
#define MATCH_VLSE256_V 0x18005007
#define MASK_VLSE256_V  0x1c00707f
#define MATCH_VLSE512_V 0x18006007
#define MASK_VLSE512_V  0x1c00707f
#define MATCH_VLSE1024_V 0x18007007
#define MASK_VLSE1024_V  0x1c00707f
#define MATCH_VSSE8_V 0x8000027
#define MASK_VSSE8_V  0x1c00707f
#define MATCH_VSSE16_V 0x8005027
#define MASK_VSSE16_V  0x1c00707f
#define MATCH_VSSE32_V 0x8006027
#define MASK_VSSE32_V  0x1c00707f
#define MATCH_VSSE64_V 0x8007027
#define MASK_VSSE64_V  0x1c00707f
#define MATCH_VSSE128_V 0x18000027
#define MASK_VSSE128_V  0x1c00707f
#define MATCH_VSSE256_V 0x18005027
#define MASK_VSSE256_V  0x1c00707f
#define MATCH_VSSE512_V 0x18006027
#define MASK_VSSE512_V  0x1c00707f
#define MATCH_VSSE1024_V 0x18007027
#define MASK_VSSE1024_V  0x1c00707f
#define MATCH_VLOXEI8_V 0xc000007
#define MASK_VLOXEI8_V  0x1c00707f
#define MATCH_VLOXEI16_V 0xc005007
#define MASK_VLOXEI16_V  0x1c00707f
#define MATCH_VLOXEI32_V 0xc006007
#define MASK_VLOXEI32_V  0x1c00707f
#define MATCH_VLOXEI64_V 0xc007007
#define MASK_VLOXEI64_V  0x1c00707f
#define MATCH_VLOXEI128_V 0x1c000007
#define MASK_VLOXEI128_V  0x1c00707f
#define MATCH_VLOXEI256_V 0x1c005007
#define MASK_VLOXEI256_V  0x1c00707f
#define MATCH_VLOXEI512_V 0x1c006007
#define MASK_VLOXEI512_V  0x1c00707f
#define MATCH_VLOXEI1024_V 0x1c007007
#define MASK_VLOXEI1024_V  0x1c00707f
#define MATCH_VSOXEI8_V 0xc000027
#define MASK_VSOXEI8_V  0x1c00707f
#define MATCH_VSOXEI16_V 0xc005027
#define MASK_VSOXEI16_V  0x1c00707f
#define MATCH_VSOXEI32_V 0xc006027
#define MASK_VSOXEI32_V  0x1c00707f
#define MATCH_VSOXEI64_V 0xc007027
#define MASK_VSOXEI64_V  0x1c00707f
#define MATCH_VSOXEI128_V 0x1c000027
#define MASK_VSOXEI128_V  0x1c00707f
#define MATCH_VSOXEI256_V 0x1c005027
#define MASK_VSOXEI256_V  0x1c00707f
#define MATCH_VSOXEI512_V 0x1c006027
#define MASK_VSOXEI512_V  0x1c00707f
#define MATCH_VSOXEI1024_V 0x1c007027
#define MASK_VSOXEI1024_V  0x1c00707f
#define MATCH_VLE8FF_V 0x1000007
#define MASK_VLE8FF_V  0x1df0707f
#define MATCH_VLE16FF_V 0x1005007
#define MASK_VLE16FF_V  0x1df0707f
#define MATCH_VLE32FF_V 0x1006007
#define MASK_VLE32FF_V  0x1df0707f
#define MATCH_VLE64FF_V 0x1007007
#define MASK_VLE64FF_V  0x1df0707f
#define MATCH_VLE128FF_V 0x11000007
#define MASK_VLE128FF_V  0x1df0707f
#define MATCH_VLE256FF_V 0x11005007
#define MASK_VLE256FF_V  0x1df0707f
#define MATCH_VLE512FF_V 0x11006007
#define MASK_VLE512FF_V  0x1df0707f
#define MATCH_VLE1024FF_V 0x11007007
#define MASK_VLE1024FF_V  0x1df0707f
#define MATCH_VL1RE8_V 0x2800007
#define MASK_VL1RE8_V  0xfff0707f
#define MATCH_VL1RE16_V 0x2805007
#define MASK_VL1RE16_V  0xfff0707f
#define MATCH_VL1RE32_V 0x2806007
#define MASK_VL1RE32_V  0xfff0707f
#define MATCH_VL1RE64_V 0x2807007
#define MASK_VL1RE64_V  0xfff0707f
#define MATCH_VL2RE8_V 0x22800007
#define MASK_VL2RE8_V  0xfff0707f
#define MATCH_VL2RE16_V 0x22805007
#define MASK_VL2RE16_V  0xfff0707f
#define MATCH_VL2RE32_V 0x22806007
#define MASK_VL2RE32_V  0xfff0707f
#define MATCH_VL2RE64_V 0x22807007
#define MASK_VL2RE64_V  0xfff0707f
#define MATCH_VL4RE8_V 0x62800007
#define MASK_VL4RE8_V  0xfff0707f
#define MATCH_VL4RE16_V 0x62805007
#define MASK_VL4RE16_V  0xfff0707f
#define MATCH_VL4RE32_V 0x62806007
#define MASK_VL4RE32_V  0xfff0707f
#define MATCH_VL4RE64_V 0x62807007
#define MASK_VL4RE64_V  0xfff0707f
#define MATCH_VL8RE8_V 0xe2800007
#define MASK_VL8RE8_V  0xfff0707f
#define MATCH_VL8RE16_V 0xe2805007
#define MASK_VL8RE16_V  0xfff0707f
#define MATCH_VL8RE32_V 0xe2806007
#define MASK_VL8RE32_V  0xfff0707f
#define MATCH_VL8RE64_V 0xe2807007
#define MASK_VL8RE64_V  0xfff0707f
#define MATCH_VS1R_V 0x2800027
#define MASK_VS1R_V  0xfff0707f
#define MATCH_VS2R_V 0x22800027
#define MASK_VS2R_V  0xfff0707f
#define MATCH_VS4R_V 0x62800027
#define MASK_VS4R_V  0xfff0707f
#define MATCH_VS8R_V 0xe2800027
#define MASK_VS8R_V  0xfff0707f
#define MATCH_VFADD_VF 0x5057
#define MASK_VFADD_VF  0xfc00707f
#define MATCH_VFSUB_VF 0x8005057
#define MASK_VFSUB_VF  0xfc00707f
#define MATCH_VFMIN_VF 0x10005057
#define MASK_VFMIN_VF  0xfc00707f
#define MATCH_VFMAX_VF 0x18005057
#define MASK_VFMAX_VF  0xfc00707f
#define MATCH_VFSGNJ_VF 0x20005057
#define MASK_VFSGNJ_VF  0xfc00707f
#define MATCH_VFSGNJN_VF 0x24005057
#define MASK_VFSGNJN_VF  0xfc00707f
#define MATCH_VFSGNJX_VF 0x28005057
#define MASK_VFSGNJX_VF  0xfc00707f
#define MATCH_VFSLIDE1UP_VF 0x38005057
#define MASK_VFSLIDE1UP_VF  0xfc00707f
#define MATCH_VFSLIDE1DOWN_VF 0x3c005057
#define MASK_VFSLIDE1DOWN_VF  0xfc00707f
#define MATCH_VFMV_S_F 0x42005057
#define MASK_VFMV_S_F  0xfff0707f
#define MATCH_VFMERGE_VFM 0x5c005057
#define MASK_VFMERGE_VFM  0xfe00707f
#define MATCH_VFMV_V_F 0x5e005057
#define MASK_VFMV_V_F  0xfff0707f
#define MATCH_VMFEQ_VF 0x60005057
#define MASK_VMFEQ_VF  0xfc00707f
#define MATCH_VMFLE_VF 0x64005057
#define MASK_VMFLE_VF  0xfc00707f
#define MATCH_VMFLT_VF 0x6c005057
#define MASK_VMFLT_VF  0xfc00707f
#define MATCH_VMFNE_VF 0x70005057
#define MASK_VMFNE_VF  0xfc00707f
#define MATCH_VMFGT_VF 0x74005057
#define MASK_VMFGT_VF  0xfc00707f
#define MATCH_VMFGE_VF 0x7c005057
#define MASK_VMFGE_VF  0xfc00707f
#define MATCH_VFDIV_VF 0x80005057
#define MASK_VFDIV_VF  0xfc00707f
#define MATCH_VFRDIV_VF 0x84005057
#define MASK_VFRDIV_VF  0xfc00707f
#define MATCH_VFMUL_VF 0x90005057
#define MASK_VFMUL_VF  0xfc00707f
#define MATCH_VFRSUB_VF 0x9c005057
#define MASK_VFRSUB_VF  0xfc00707f
#define MATCH_VFMADD_VF 0xa0005057
#define MASK_VFMADD_VF  0xfc00707f
#define MATCH_VFNMADD_VF 0xa4005057
#define MASK_VFNMADD_VF  0xfc00707f
#define MATCH_VFMSUB_VF 0xa8005057
#define MASK_VFMSUB_VF  0xfc00707f
#define MATCH_VFNMSUB_VF 0xac005057
#define MASK_VFNMSUB_VF  0xfc00707f
#define MATCH_VFMACC_VF 0xb0005057
#define MASK_VFMACC_VF  0xfc00707f
#define MATCH_VFNMACC_VF 0xb4005057
#define MASK_VFNMACC_VF  0xfc00707f
#define MATCH_VFMSAC_VF 0xb8005057
#define MASK_VFMSAC_VF  0xfc00707f
#define MATCH_VFNMSAC_VF 0xbc005057
#define MASK_VFNMSAC_VF  0xfc00707f
#define MATCH_VFWADD_VF 0xc0005057
#define MASK_VFWADD_VF  0xfc00707f
#define MATCH_VFWSUB_VF 0xc8005057
#define MASK_VFWSUB_VF  0xfc00707f
#define MATCH_VFWADD_WF 0xd0005057
#define MASK_VFWADD_WF  0xfc00707f
#define MATCH_VFWSUB_WF 0xd8005057
#define MASK_VFWSUB_WF  0xfc00707f
#define MATCH_VFWMUL_VF 0xe0005057
#define MASK_VFWMUL_VF  0xfc00707f
#define MATCH_VFWMACC_VF 0xf0005057
#define MASK_VFWMACC_VF  0xfc00707f
#define MATCH_VFWNMACC_VF 0xf4005057
#define MASK_VFWNMACC_VF  0xfc00707f
#define MATCH_VFWMSAC_VF 0xf8005057
#define MASK_VFWMSAC_VF  0xfc00707f
#define MATCH_VFWNMSAC_VF 0xfc005057
#define MASK_VFWNMSAC_VF  0xfc00707f
#define MATCH_VFADD_VV 0x1057
#define MASK_VFADD_VV  0xfc00707f
#define MATCH_VFREDSUM_VS 0x4001057
#define MASK_VFREDSUM_VS  0xfc00707f
#define MATCH_VFSUB_VV 0x8001057
#define MASK_VFSUB_VV  0xfc00707f
#define MATCH_VFREDOSUM_VS 0xc001057
#define MASK_VFREDOSUM_VS  0xfc00707f
#define MATCH_VFMIN_VV 0x10001057
#define MASK_VFMIN_VV  0xfc00707f
#define MATCH_VFREDMIN_VS 0x14001057
#define MASK_VFREDMIN_VS  0xfc00707f
#define MATCH_VFMAX_VV 0x18001057
#define MASK_VFMAX_VV  0xfc00707f
#define MATCH_VFREDMAX_VS 0x1c001057
#define MASK_VFREDMAX_VS  0xfc00707f
#define MATCH_VFSGNJ_VV 0x20001057
#define MASK_VFSGNJ_VV  0xfc00707f
#define MATCH_VFSGNJN_VV 0x24001057
#define MASK_VFSGNJN_VV  0xfc00707f
#define MATCH_VFSGNJX_VV 0x28001057
#define MASK_VFSGNJX_VV  0xfc00707f
#define MATCH_VFMV_F_S 0x42001057
#define MASK_VFMV_F_S  0xfe0ff07f
#define MATCH_VMFEQ_VV 0x60001057
#define MASK_VMFEQ_VV  0xfc00707f
#define MATCH_VMFLE_VV 0x64001057
#define MASK_VMFLE_VV  0xfc00707f
#define MATCH_VMFLT_VV 0x6c001057
#define MASK_VMFLT_VV  0xfc00707f
#define MATCH_VMFNE_VV 0x70001057
#define MASK_VMFNE_VV  0xfc00707f
#define MATCH_VFDIV_VV 0x80001057
#define MASK_VFDIV_VV  0xfc00707f
#define MATCH_VFMUL_VV 0x90001057
#define MASK_VFMUL_VV  0xfc00707f
#define MATCH_VFMADD_VV 0xa0001057
#define MASK_VFMADD_VV  0xfc00707f
#define MATCH_VFNMADD_VV 0xa4001057
#define MASK_VFNMADD_VV  0xfc00707f
#define MATCH_VFMSUB_VV 0xa8001057
#define MASK_VFMSUB_VV  0xfc00707f
#define MATCH_VFNMSUB_VV 0xac001057
#define MASK_VFNMSUB_VV  0xfc00707f
#define MATCH_VFMACC_VV 0xb0001057
#define MASK_VFMACC_VV  0xfc00707f
#define MATCH_VFNMACC_VV 0xb4001057
#define MASK_VFNMACC_VV  0xfc00707f
#define MATCH_VFMSAC_VV 0xb8001057
#define MASK_VFMSAC_VV  0xfc00707f
#define MATCH_VFNMSAC_VV 0xbc001057
#define MASK_VFNMSAC_VV  0xfc00707f
#define MATCH_VFCVT_XU_F_V 0x48001057
#define MASK_VFCVT_XU_F_V  0xfc0ff07f
#define MATCH_VFCVT_X_F_V 0x48009057
#define MASK_VFCVT_X_F_V  0xfc0ff07f
#define MATCH_VFCVT_F_XU_V 0x48011057
#define MASK_VFCVT_F_XU_V  0xfc0ff07f
#define MATCH_VFCVT_F_X_V 0x48019057
#define MASK_VFCVT_F_X_V  0xfc0ff07f
#define MATCH_VFCVT_RTZ_XU_F_V 0x48031057
#define MASK_VFCVT_RTZ_XU_F_V  0xfc0ff07f
#define MATCH_VFCVT_RTZ_X_F_V 0x48039057
#define MASK_VFCVT_RTZ_X_F_V  0xfc0ff07f
#define MATCH_VFWCVT_XU_F_V 0x48041057
#define MASK_VFWCVT_XU_F_V  0xfc0ff07f
#define MATCH_VFWCVT_X_F_V 0x48049057
#define MASK_VFWCVT_X_F_V  0xfc0ff07f
#define MATCH_VFWCVT_F_XU_V 0x48051057
#define MASK_VFWCVT_F_XU_V  0xfc0ff07f
#define MATCH_VFWCVT_F_X_V 0x48059057
#define MASK_VFWCVT_F_X_V  0xfc0ff07f
#define MATCH_VFWCVT_F_F_V 0x48061057
#define MASK_VFWCVT_F_F_V  0xfc0ff07f
#define MATCH_VFWCVT_RTZ_XU_F_V 0x48071057
#define MASK_VFWCVT_RTZ_XU_F_V  0xfc0ff07f
#define MATCH_VFWCVT_RTZ_X_F_V 0x48079057
#define MASK_VFWCVT_RTZ_X_F_V  0xfc0ff07f
#define MATCH_VFNCVT_XU_F_W 0x48081057
#define MASK_VFNCVT_XU_F_W  0xfc0ff07f
#define MATCH_VFNCVT_X_F_W 0x48089057
#define MASK_VFNCVT_X_F_W  0xfc0ff07f
#define MATCH_VFNCVT_F_XU_W 0x48091057
#define MASK_VFNCVT_F_XU_W  0xfc0ff07f
#define MATCH_VFNCVT_F_X_W 0x48099057
#define MASK_VFNCVT_F_X_W  0xfc0ff07f
#define MATCH_VFNCVT_F_F_W 0x480a1057
#define MASK_VFNCVT_F_F_W  0xfc0ff07f
#define MATCH_VFNCVT_ROD_F_F_W 0x480a9057
#define MASK_VFNCVT_ROD_F_F_W  0xfc0ff07f
#define MATCH_VFNCVT_RTZ_XU_F_W 0x480b1057
#define MASK_VFNCVT_RTZ_XU_F_W  0xfc0ff07f
#define MATCH_VFNCVT_RTZ_X_F_W 0x480b9057
#define MASK_VFNCVT_RTZ_X_F_W  0xfc0ff07f
#define MATCH_VFSQRT_V 0x4c001057
#define MASK_VFSQRT_V  0xfc0ff07f
#define MATCH_VFRSQRTE7_V 0x4c021057
#define MASK_VFRSQRTE7_V  0xfc0ff07f
#define MATCH_VFRECE7_V 0x4c029057
#define MASK_VFRECE7_V  0xfc0ff07f
#define MATCH_VFCLASS_V 0x4c081057
#define MASK_VFCLASS_V  0xfc0ff07f
#define MATCH_VFWADD_VV 0xc0001057
#define MASK_VFWADD_VV  0xfc00707f
#define MATCH_VFWREDSUM_VS 0xc4001057
#define MASK_VFWREDSUM_VS  0xfc00707f
#define MATCH_VFWSUB_VV 0xc8001057
#define MASK_VFWSUB_VV  0xfc00707f
#define MATCH_VFWREDOSUM_VS 0xcc001057
#define MASK_VFWREDOSUM_VS  0xfc00707f
#define MATCH_VFWADD_WV 0xd0001057
#define MASK_VFWADD_WV  0xfc00707f
#define MATCH_VFWSUB_WV 0xd8001057
#define MASK_VFWSUB_WV  0xfc00707f
#define MATCH_VFWMUL_VV 0xe0001057
#define MASK_VFWMUL_VV  0xfc00707f
#define MATCH_VFDOT_VV 0xe4001057
#define MASK_VFDOT_VV  0xfc00707f
#define MATCH_VFWMACC_VV 0xf0001057
#define MASK_VFWMACC_VV  0xfc00707f
#define MATCH_VFWNMACC_VV 0xf4001057
#define MASK_VFWNMACC_VV  0xfc00707f
#define MATCH_VFWMSAC_VV 0xf8001057
#define MASK_VFWMSAC_VV  0xfc00707f
#define MATCH_VFWNMSAC_VV 0xfc001057
#define MASK_VFWNMSAC_VV  0xfc00707f
#define MATCH_VADD_VX 0x4057
#define MASK_VADD_VX  0xfc00707f
#define MATCH_VSUB_VX 0x8004057
#define MASK_VSUB_VX  0xfc00707f
#define MATCH_VRSUB_VX 0xc004057
#define MASK_VRSUB_VX  0xfc00707f
#define MATCH_VMINU_VX 0x10004057
#define MASK_VMINU_VX  0xfc00707f
#define MATCH_VMIN_VX 0x14004057
#define MASK_VMIN_VX  0xfc00707f
#define MATCH_VMAXU_VX 0x18004057
#define MASK_VMAXU_VX  0xfc00707f
#define MATCH_VMAX_VX 0x1c004057
#define MASK_VMAX_VX  0xfc00707f
#define MATCH_VAND_VX 0x24004057
#define MASK_VAND_VX  0xfc00707f
#define MATCH_VOR_VX 0x28004057
#define MASK_VOR_VX  0xfc00707f
#define MATCH_VXOR_VX 0x2c004057
#define MASK_VXOR_VX  0xfc00707f
#define MATCH_VRGATHER_VX 0x30004057
#define MASK_VRGATHER_VX  0xfc00707f
#define MATCH_VSLIDEUP_VX 0x38004057
#define MASK_VSLIDEUP_VX  0xfc00707f
#define MATCH_VSLIDEDOWN_VX 0x3c004057
#define MASK_VSLIDEDOWN_VX  0xfc00707f
#define MATCH_VADC_VXM 0x40004057
#define MASK_VADC_VXM  0xfe00707f
#define MATCH_VMADC_VXM 0x44004057
#define MASK_VMADC_VXM  0xfc00707f
#define MATCH_VSBC_VXM 0x48004057
#define MASK_VSBC_VXM  0xfe00707f
#define MATCH_VMSBC_VXM 0x4c004057
#define MASK_VMSBC_VXM  0xfc00707f
#define MATCH_VMERGE_VXM 0x5c004057
#define MASK_VMERGE_VXM  0xfe00707f
#define MATCH_VMV_V_X 0x5e004057
#define MASK_VMV_V_X  0xfff0707f
#define MATCH_VMSEQ_VX 0x60004057
#define MASK_VMSEQ_VX  0xfc00707f
#define MATCH_VMSNE_VX 0x64004057
#define MASK_VMSNE_VX  0xfc00707f
#define MATCH_VMSLTU_VX 0x68004057
#define MASK_VMSLTU_VX  0xfc00707f
#define MATCH_VMSLT_VX 0x6c004057
#define MASK_VMSLT_VX  0xfc00707f
#define MATCH_VMSLEU_VX 0x70004057
#define MASK_VMSLEU_VX  0xfc00707f
#define MATCH_VMSLE_VX 0x74004057
#define MASK_VMSLE_VX  0xfc00707f
#define MATCH_VMSGTU_VX 0x78004057
#define MASK_VMSGTU_VX  0xfc00707f
#define MATCH_VMSGT_VX 0x7c004057
#define MASK_VMSGT_VX  0xfc00707f
#define MATCH_VSADDU_VX 0x80004057
#define MASK_VSADDU_VX  0xfc00707f
#define MATCH_VSADD_VX 0x84004057
#define MASK_VSADD_VX  0xfc00707f
#define MATCH_VSSUBU_VX 0x88004057
#define MASK_VSSUBU_VX  0xfc00707f
#define MATCH_VSSUB_VX 0x8c004057
#define MASK_VSSUB_VX  0xfc00707f
#define MATCH_VSLL_VX 0x94004057
#define MASK_VSLL_VX  0xfc00707f
#define MATCH_VSMUL_VX 0x9c004057
#define MASK_VSMUL_VX  0xfc00707f
#define MATCH_VSRL_VX 0xa0004057
#define MASK_VSRL_VX  0xfc00707f
#define MATCH_VSRA_VX 0xa4004057
#define MASK_VSRA_VX  0xfc00707f
#define MATCH_VSSRL_VX 0xa8004057
#define MASK_VSSRL_VX  0xfc00707f
#define MATCH_VSSRA_VX 0xac004057
#define MASK_VSSRA_VX  0xfc00707f
#define MATCH_VNSRL_WX 0xb0004057
#define MASK_VNSRL_WX  0xfc00707f
#define MATCH_VNSRA_WX 0xb4004057
#define MASK_VNSRA_WX  0xfc00707f
#define MATCH_VNCLIPU_WX 0xb8004057
#define MASK_VNCLIPU_WX  0xfc00707f
#define MATCH_VNCLIP_WX 0xbc004057
#define MASK_VNCLIP_WX  0xfc00707f
#define MATCH_VQMACCU_VX 0xf0004057
#define MASK_VQMACCU_VX  0xfc00707f
#define MATCH_VQMACC_VX 0xf4004057
#define MASK_VQMACC_VX  0xfc00707f
#define MATCH_VQMACCUS_VX 0xf8004057
#define MASK_VQMACCUS_VX  0xfc00707f
#define MATCH_VQMACCSU_VX 0xfc004057
#define MASK_VQMACCSU_VX  0xfc00707f
#define MATCH_VADD_VV 0x57
#define MASK_VADD_VV  0xfc00707f
#define MATCH_VSUB_VV 0x8000057
#define MASK_VSUB_VV  0xfc00707f
#define MATCH_VMINU_VV 0x10000057
#define MASK_VMINU_VV  0xfc00707f
#define MATCH_VMIN_VV 0x14000057
#define MASK_VMIN_VV  0xfc00707f
#define MATCH_VMAXU_VV 0x18000057
#define MASK_VMAXU_VV  0xfc00707f
#define MATCH_VMAX_VV 0x1c000057
#define MASK_VMAX_VV  0xfc00707f
#define MATCH_VAND_VV 0x24000057
#define MASK_VAND_VV  0xfc00707f
#define MATCH_VOR_VV 0x28000057
#define MASK_VOR_VV  0xfc00707f
#define MATCH_VXOR_VV 0x2c000057
#define MASK_VXOR_VV  0xfc00707f
#define MATCH_VRGATHER_VV 0x30000057
#define MASK_VRGATHER_VV  0xfc00707f
#define MATCH_VRGATHEREI16_VV 0x38000057
#define MASK_VRGATHEREI16_VV  0xfc00707f
#define MATCH_VADC_VVM 0x40000057
#define MASK_VADC_VVM  0xfe00707f
#define MATCH_VMADC_VVM 0x44000057
#define MASK_VMADC_VVM  0xfc00707f
#define MATCH_VSBC_VVM 0x48000057
#define MASK_VSBC_VVM  0xfe00707f
#define MATCH_VMSBC_VVM 0x4c000057
#define MASK_VMSBC_VVM  0xfc00707f
#define MATCH_VMERGE_VVM 0x5c000057
#define MASK_VMERGE_VVM  0xfe00707f
#define MATCH_VMV_V_V 0x5e000057
#define MASK_VMV_V_V  0xfff0707f
#define MATCH_VMSEQ_VV 0x60000057
#define MASK_VMSEQ_VV  0xfc00707f
#define MATCH_VMSNE_VV 0x64000057
#define MASK_VMSNE_VV  0xfc00707f
#define MATCH_VMSLTU_VV 0x68000057
#define MASK_VMSLTU_VV  0xfc00707f
#define MATCH_VMSLT_VV 0x6c000057
#define MASK_VMSLT_VV  0xfc00707f
#define MATCH_VMSLEU_VV 0x70000057
#define MASK_VMSLEU_VV  0xfc00707f
#define MATCH_VMSLE_VV 0x74000057
#define MASK_VMSLE_VV  0xfc00707f
#define MATCH_VSADDU_VV 0x80000057
#define MASK_VSADDU_VV  0xfc00707f
#define MATCH_VSADD_VV 0x84000057
#define MASK_VSADD_VV  0xfc00707f
#define MATCH_VSSUBU_VV 0x88000057
#define MASK_VSSUBU_VV  0xfc00707f
#define MATCH_VSSUB_VV 0x8c000057
#define MASK_VSSUB_VV  0xfc00707f
#define MATCH_VSLL_VV 0x94000057
#define MASK_VSLL_VV  0xfc00707f
#define MATCH_VSMUL_VV 0x9c000057
#define MASK_VSMUL_VV  0xfc00707f
#define MATCH_VSRL_VV 0xa0000057
#define MASK_VSRL_VV  0xfc00707f
#define MATCH_VSRA_VV 0xa4000057
#define MASK_VSRA_VV  0xfc00707f
#define MATCH_VSSRL_VV 0xa8000057
#define MASK_VSSRL_VV  0xfc00707f
#define MATCH_VSSRA_VV 0xac000057
#define MASK_VSSRA_VV  0xfc00707f
#define MATCH_VNSRL_WV 0xb0000057
#define MASK_VNSRL_WV  0xfc00707f
#define MATCH_VNSRA_WV 0xb4000057
#define MASK_VNSRA_WV  0xfc00707f
#define MATCH_VNCLIPU_WV 0xb8000057
#define MASK_VNCLIPU_WV  0xfc00707f
#define MATCH_VNCLIP_WV 0xbc000057
#define MASK_VNCLIP_WV  0xfc00707f
#define MATCH_VWREDSUMU_VS 0xc0000057
#define MASK_VWREDSUMU_VS  0xfc00707f
#define MATCH_VWREDSUM_VS 0xc4000057
#define MASK_VWREDSUM_VS  0xfc00707f
#define MATCH_VDOTU_VV 0xe0000057
#define MASK_VDOTU_VV  0xfc00707f
#define MATCH_VDOT_VV 0xe4000057
#define MASK_VDOT_VV  0xfc00707f
#define MATCH_VQMACCU_VV 0xf0000057
#define MASK_VQMACCU_VV  0xfc00707f
#define MATCH_VQMACC_VV 0xf4000057
#define MASK_VQMACC_VV  0xfc00707f
#define MATCH_VQMACCSU_VV 0xfc000057
#define MASK_VQMACCSU_VV  0xfc00707f
#define MATCH_VADD_VI 0x3057
#define MASK_VADD_VI  0xfc00707f
#define MATCH_VRSUB_VI 0xc003057
#define MASK_VRSUB_VI  0xfc00707f
#define MATCH_VAND_VI 0x24003057
#define MASK_VAND_VI  0xfc00707f
#define MATCH_VOR_VI 0x28003057
#define MASK_VOR_VI  0xfc00707f
#define MATCH_VXOR_VI 0x2c003057
#define MASK_VXOR_VI  0xfc00707f
#define MATCH_VRGATHER_VI 0x30003057
#define MASK_VRGATHER_VI  0xfc00707f
#define MATCH_VSLIDEUP_VI 0x38003057
#define MASK_VSLIDEUP_VI  0xfc00707f
#define MATCH_VSLIDEDOWN_VI 0x3c003057
#define MASK_VSLIDEDOWN_VI  0xfc00707f
#define MATCH_VADC_VIM 0x40003057
#define MASK_VADC_VIM  0xfe00707f
#define MATCH_VMADC_VIM 0x44003057
#define MASK_VMADC_VIM  0xfc00707f
#define MATCH_VMERGE_VIM 0x5c003057
#define MASK_VMERGE_VIM  0xfe00707f
#define MATCH_VMV_V_I 0x5e003057
#define MASK_VMV_V_I  0xfff0707f
#define MATCH_VMSEQ_VI 0x60003057
#define MASK_VMSEQ_VI  0xfc00707f
#define MATCH_VMSNE_VI 0x64003057
#define MASK_VMSNE_VI  0xfc00707f
#define MATCH_VMSLEU_VI 0x70003057
#define MASK_VMSLEU_VI  0xfc00707f
#define MATCH_VMSLE_VI 0x74003057
#define MASK_VMSLE_VI  0xfc00707f
#define MATCH_VMSGTU_VI 0x78003057
#define MASK_VMSGTU_VI  0xfc00707f
#define MATCH_VMSGT_VI 0x7c003057
#define MASK_VMSGT_VI  0xfc00707f
#define MATCH_VSADDU_VI 0x80003057
#define MASK_VSADDU_VI  0xfc00707f
#define MATCH_VSADD_VI 0x84003057
#define MASK_VSADD_VI  0xfc00707f
#define MATCH_VSLL_VI 0x94003057
#define MASK_VSLL_VI  0xfc00707f
#define MATCH_VMV1R_V 0x9e003057
#define MASK_VMV1R_V  0xfe0ff07f
#define MATCH_VMV2R_V 0x9e00b057
#define MASK_VMV2R_V  0xfe0ff07f
#define MATCH_VMV4R_V 0x9e01b057
#define MASK_VMV4R_V  0xfe0ff07f
#define MATCH_VMV8R_V 0x9e03b057
#define MASK_VMV8R_V  0xfe0ff07f
#define MATCH_VSRL_VI 0xa0003057
#define MASK_VSRL_VI  0xfc00707f
#define MATCH_VSRA_VI 0xa4003057
#define MASK_VSRA_VI  0xfc00707f
#define MATCH_VSSRL_VI 0xa8003057
#define MASK_VSSRL_VI  0xfc00707f
#define MATCH_VSSRA_VI 0xac003057
#define MASK_VSSRA_VI  0xfc00707f
#define MATCH_VNSRL_WI 0xb0003057
#define MASK_VNSRL_WI  0xfc00707f
#define MATCH_VNSRA_WI 0xb4003057
#define MASK_VNSRA_WI  0xfc00707f
#define MATCH_VNCLIPU_WI 0xb8003057
#define MASK_VNCLIPU_WI  0xfc00707f
#define MATCH_VNCLIP_WI 0xbc003057
#define MASK_VNCLIP_WI  0xfc00707f
#define MATCH_VREDSUM_VS 0x2057
#define MASK_VREDSUM_VS  0xfc00707f
#define MATCH_VREDAND_VS 0x4002057
#define MASK_VREDAND_VS  0xfc00707f
#define MATCH_VREDOR_VS 0x8002057
#define MASK_VREDOR_VS  0xfc00707f
#define MATCH_VREDXOR_VS 0xc002057
#define MASK_VREDXOR_VS  0xfc00707f
#define MATCH_VREDMINU_VS 0x10002057
#define MASK_VREDMINU_VS  0xfc00707f
#define MATCH_VREDMIN_VS 0x14002057
#define MASK_VREDMIN_VS  0xfc00707f
#define MATCH_VREDMAXU_VS 0x18002057
#define MASK_VREDMAXU_VS  0xfc00707f
#define MATCH_VREDMAX_VS 0x1c002057
#define MASK_VREDMAX_VS  0xfc00707f
#define MATCH_VAADDU_VV 0x20002057
#define MASK_VAADDU_VV  0xfc00707f
#define MATCH_VAADD_VV 0x24002057
#define MASK_VAADD_VV  0xfc00707f
#define MATCH_VASUBU_VV 0x28002057
#define MASK_VASUBU_VV  0xfc00707f
#define MATCH_VASUB_VV 0x2c002057
#define MASK_VASUB_VV  0xfc00707f
#define MATCH_VMV_X_S 0x42002057
#define MASK_VMV_X_S  0xfe0ff07f
#define MATCH_VZEXT_VF8 0x48012057
#define MASK_VZEXT_VF8  0xfc0ff07f
#define MATCH_VSEXT_VF8 0x4801a057
#define MASK_VSEXT_VF8  0xfc0ff07f
#define MATCH_VZEXT_VF4 0x48022057
#define MASK_VZEXT_VF4  0xfc0ff07f
#define MATCH_VSEXT_VF4 0x4802a057
#define MASK_VSEXT_VF4  0xfc0ff07f
#define MATCH_VZEXT_VF2 0x48032057
#define MASK_VZEXT_VF2  0xfc0ff07f
#define MATCH_VSEXT_VF2 0x4803a057
#define MASK_VSEXT_VF2  0xfc0ff07f
#define MATCH_VCOMPRESS_VM 0x5e002057
#define MASK_VCOMPRESS_VM  0xfe00707f
#define MATCH_VMANDNOT_MM 0x60002057
#define MASK_VMANDNOT_MM  0xfc00707f
#define MATCH_VMAND_MM 0x64002057
#define MASK_VMAND_MM  0xfc00707f
#define MATCH_VMOR_MM 0x68002057
#define MASK_VMOR_MM  0xfc00707f
#define MATCH_VMXOR_MM 0x6c002057
#define MASK_VMXOR_MM  0xfc00707f
#define MATCH_VMORNOT_MM 0x70002057
#define MASK_VMORNOT_MM  0xfc00707f
#define MATCH_VMNAND_MM 0x74002057
#define MASK_VMNAND_MM  0xfc00707f
#define MATCH_VMNOR_MM 0x78002057
#define MASK_VMNOR_MM  0xfc00707f
#define MATCH_VMXNOR_MM 0x7c002057
#define MASK_VMXNOR_MM  0xfc00707f
#define MATCH_VMSBF_M 0x5000a057
#define MASK_VMSBF_M  0xfc0ff07f
#define MATCH_VMSOF_M 0x50012057
#define MASK_VMSOF_M  0xfc0ff07f
#define MATCH_VMSIF_M 0x5001a057
#define MASK_VMSIF_M  0xfc0ff07f
#define MATCH_VIOTA_M 0x50082057
#define MASK_VIOTA_M  0xfc0ff07f
#define MATCH_VID_V 0x5008a057
#define MASK_VID_V  0xfdfff07f
#define MATCH_VPOPC_M 0x40082057
#define MASK_VPOPC_M  0xfc0ff07f
#define MATCH_VFIRST_M 0x4008a057
#define MASK_VFIRST_M  0xfc0ff07f
#define MATCH_VDIVU_VV 0x80002057
#define MASK_VDIVU_VV  0xfc00707f
#define MATCH_VDIV_VV 0x84002057
#define MASK_VDIV_VV  0xfc00707f
#define MATCH_VREMU_VV 0x88002057
#define MASK_VREMU_VV  0xfc00707f
#define MATCH_VREM_VV 0x8c002057
#define MASK_VREM_VV  0xfc00707f
#define MATCH_VMULHU_VV 0x90002057
#define MASK_VMULHU_VV  0xfc00707f
#define MATCH_VMUL_VV 0x94002057
#define MASK_VMUL_VV  0xfc00707f
#define MATCH_VMULHSU_VV 0x98002057
#define MASK_VMULHSU_VV  0xfc00707f
#define MATCH_VMULH_VV 0x9c002057
#define MASK_VMULH_VV  0xfc00707f
#define MATCH_VMADD_VV 0xa4002057
#define MASK_VMADD_VV  0xfc00707f
#define MATCH_VNMSUB_VV 0xac002057
#define MASK_VNMSUB_VV  0xfc00707f
#define MATCH_VMACC_VV 0xb4002057
#define MASK_VMACC_VV  0xfc00707f
#define MATCH_VNMSAC_VV 0xbc002057
#define MASK_VNMSAC_VV  0xfc00707f
#define MATCH_VWADDU_VV 0xc0002057
#define MASK_VWADDU_VV  0xfc00707f
#define MATCH_VWADD_VV 0xc4002057
#define MASK_VWADD_VV  0xfc00707f
#define MATCH_VWSUBU_VV 0xc8002057
#define MASK_VWSUBU_VV  0xfc00707f
#define MATCH_VWSUB_VV 0xcc002057
#define MASK_VWSUB_VV  0xfc00707f
#define MATCH_VWADDU_WV 0xd0002057
#define MASK_VWADDU_WV  0xfc00707f
#define MATCH_VWADD_WV 0xd4002057
#define MASK_VWADD_WV  0xfc00707f
#define MATCH_VWSUBU_WV 0xd8002057
#define MASK_VWSUBU_WV  0xfc00707f
#define MATCH_VWSUB_WV 0xdc002057
#define MASK_VWSUB_WV  0xfc00707f
#define MATCH_VWMULU_VV 0xe0002057
#define MASK_VWMULU_VV  0xfc00707f
#define MATCH_VWMULSU_VV 0xe8002057
#define MASK_VWMULSU_VV  0xfc00707f
#define MATCH_VWMUL_VV 0xec002057
#define MASK_VWMUL_VV  0xfc00707f
#define MATCH_VWMACCU_VV 0xf0002057
#define MASK_VWMACCU_VV  0xfc00707f
#define MATCH_VWMACC_VV 0xf4002057
#define MASK_VWMACC_VV  0xfc00707f
#define MATCH_VWMACCSU_VV 0xfc002057
#define MASK_VWMACCSU_VV  0xfc00707f
#define MATCH_VAADDU_VX 0x20006057
#define MASK_VAADDU_VX  0xfc00707f
#define MATCH_VAADD_VX 0x24006057
#define MASK_VAADD_VX  0xfc00707f
#define MATCH_VASUBU_VX 0x28006057
#define MASK_VASUBU_VX  0xfc00707f
#define MATCH_VASUB_VX 0x2c006057
#define MASK_VASUB_VX  0xfc00707f
#define MATCH_VMV_S_X 0x42006057
#define MASK_VMV_S_X  0xfff0707f
#define MATCH_VSLIDE1UP_VX 0x38006057
#define MASK_VSLIDE1UP_VX  0xfc00707f
#define MATCH_VSLIDE1DOWN_VX 0x3c006057
#define MASK_VSLIDE1DOWN_VX  0xfc00707f
#define MATCH_VDIVU_VX 0x80006057
#define MASK_VDIVU_VX  0xfc00707f
#define MATCH_VDIV_VX 0x84006057
#define MASK_VDIV_VX  0xfc00707f
#define MATCH_VREMU_VX 0x88006057
#define MASK_VREMU_VX  0xfc00707f
#define MATCH_VREM_VX 0x8c006057
#define MASK_VREM_VX  0xfc00707f
#define MATCH_VMULHU_VX 0x90006057
#define MASK_VMULHU_VX  0xfc00707f
#define MATCH_VMUL_VX 0x94006057
#define MASK_VMUL_VX  0xfc00707f
#define MATCH_VMULHSU_VX 0x98006057
#define MASK_VMULHSU_VX  0xfc00707f
#define MATCH_VMULH_VX 0x9c006057
#define MASK_VMULH_VX  0xfc00707f
#define MATCH_VMADD_VX 0xa4006057
#define MASK_VMADD_VX  0xfc00707f
#define MATCH_VNMSUB_VX 0xac006057
#define MASK_VNMSUB_VX  0xfc00707f
#define MATCH_VMACC_VX 0xb4006057
#define MASK_VMACC_VX  0xfc00707f
#define MATCH_VNMSAC_VX 0xbc006057
#define MASK_VNMSAC_VX  0xfc00707f
#define MATCH_VWADDU_VX 0xc0006057
#define MASK_VWADDU_VX  0xfc00707f
#define MATCH_VWADD_VX 0xc4006057
#define MASK_VWADD_VX  0xfc00707f
#define MATCH_VWSUBU_VX 0xc8006057
#define MASK_VWSUBU_VX  0xfc00707f
#define MATCH_VWSUB_VX 0xcc006057
#define MASK_VWSUB_VX  0xfc00707f
#define MATCH_VWADDU_WX 0xd0006057
#define MASK_VWADDU_WX  0xfc00707f
#define MATCH_VWADD_WX 0xd4006057
#define MASK_VWADD_WX  0xfc00707f
#define MATCH_VWSUBU_WX 0xd8006057
#define MASK_VWSUBU_WX  0xfc00707f
#define MATCH_VWSUB_WX 0xdc006057
#define MASK_VWSUB_WX  0xfc00707f
#define MATCH_VWMULU_VX 0xe0006057
#define MASK_VWMULU_VX  0xfc00707f
#define MATCH_VWMULSU_VX 0xe8006057
#define MASK_VWMULSU_VX  0xfc00707f
#define MATCH_VWMUL_VX 0xec006057
#define MASK_VWMUL_VX  0xfc00707f
#define MATCH_VWMACCU_VX 0xf0006057
#define MASK_VWMACCU_VX  0xfc00707f
#define MATCH_VWMACC_VX 0xf4006057
#define MASK_VWMACC_VX  0xfc00707f
#define MATCH_VWMACCUS_VX 0xf8006057
#define MASK_VWMACCUS_VX  0xfc00707f
#define MATCH_VWMACCSU_VX 0xfc006057
#define MASK_VWMACCSU_VX  0xfc00707f
#define MATCH_VAMOSWAPEI8_V 0x800002f
#define MASK_VAMOSWAPEI8_V  0xf800707f
#define MATCH_VAMOADDEI8_V 0x2f
#define MASK_VAMOADDEI8_V  0xf800707f
#define MATCH_VAMOXOREI8_V 0x2000002f
#define MASK_VAMOXOREI8_V  0xf800707f
#define MATCH_VAMOANDEI8_V 0x6000002f
#define MASK_VAMOANDEI8_V  0xf800707f
#define MATCH_VAMOOREI8_V 0x4000002f
#define MASK_VAMOOREI8_V  0xf800707f
#define MATCH_VAMOMINEI8_V 0x8000002f
#define MASK_VAMOMINEI8_V  0xf800707f
#define MATCH_VAMOMAXEI8_V 0xa000002f
#define MASK_VAMOMAXEI8_V  0xf800707f
#define MATCH_VAMOMINUEI8_V 0xc000002f
#define MASK_VAMOMINUEI8_V  0xf800707f
#define MATCH_VAMOMAXUEI8_V 0xe000002f
#define MASK_VAMOMAXUEI8_V  0xf800707f
#define MATCH_VAMOSWAPEI16_V 0x800502f
#define MASK_VAMOSWAPEI16_V  0xf800707f
#define MATCH_VAMOADDEI16_V 0x502f
#define MASK_VAMOADDEI16_V  0xf800707f
#define MATCH_VAMOXOREI16_V 0x2000502f
#define MASK_VAMOXOREI16_V  0xf800707f
#define MATCH_VAMOANDEI16_V 0x6000502f
#define MASK_VAMOANDEI16_V  0xf800707f
#define MATCH_VAMOOREI16_V 0x4000502f
#define MASK_VAMOOREI16_V  0xf800707f
#define MATCH_VAMOMINEI16_V 0x8000502f
#define MASK_VAMOMINEI16_V  0xf800707f
#define MATCH_VAMOMAXEI16_V 0xa000502f
#define MASK_VAMOMAXEI16_V  0xf800707f
#define MATCH_VAMOMINUEI16_V 0xc000502f
#define MASK_VAMOMINUEI16_V  0xf800707f
#define MATCH_VAMOMAXUEI16_V 0xe000502f
#define MASK_VAMOMAXUEI16_V  0xf800707f
#define MATCH_VAMOSWAPEI32_V 0x800602f
#define MASK_VAMOSWAPEI32_V  0xf800707f
#define MATCH_VAMOADDEI32_V 0x602f
#define MASK_VAMOADDEI32_V  0xf800707f
#define MATCH_VAMOXOREI32_V 0x2000602f
#define MASK_VAMOXOREI32_V  0xf800707f
#define MATCH_VAMOANDEI32_V 0x6000602f
#define MASK_VAMOANDEI32_V  0xf800707f
#define MATCH_VAMOOREI32_V 0x4000602f
#define MASK_VAMOOREI32_V  0xf800707f
#define MATCH_VAMOMINEI32_V 0x8000602f
#define MASK_VAMOMINEI32_V  0xf800707f
#define MATCH_VAMOMAXEI32_V 0xa000602f
#define MASK_VAMOMAXEI32_V  0xf800707f
#define MATCH_VAMOMINUEI32_V 0xc000602f
#define MASK_VAMOMINUEI32_V  0xf800707f
#define MATCH_VAMOMAXUEI32_V 0xe000602f
#define MASK_VAMOMAXUEI32_V  0xf800707f
#define MATCH_VAMOSWAPEI64_V 0x800702f
#define MASK_VAMOSWAPEI64_V  0xf800707f
#define MATCH_VAMOADDEI64_V 0x702f
#define MASK_VAMOADDEI64_V  0xf800707f
#define MATCH_VAMOXOREI64_V 0x2000702f
#define MASK_VAMOXOREI64_V  0xf800707f
#define MATCH_VAMOANDEI64_V 0x6000702f
#define MASK_VAMOANDEI64_V  0xf800707f
#define MATCH_VAMOOREI64_V 0x4000702f
#define MASK_VAMOOREI64_V  0xf800707f
#define MATCH_VAMOMINEI64_V 0x8000702f
#define MASK_VAMOMINEI64_V  0xf800707f
#define MATCH_VAMOMAXEI64_V 0xa000702f
#define MASK_VAMOMAXEI64_V  0xf800707f
#define MATCH_VAMOMINUEI64_V 0xc000702f
#define MASK_VAMOMINUEI64_V  0xf800707f
#define MATCH_VAMOMAXUEI64_V 0xe000702f
#define MASK_VAMOMAXUEI64_V  0xf800707f
#define MATCH_VMVNFR_V 0x9e003057
#define MASK_VMVNFR_V  0xfe00707f
#define MATCH_VL1R_V 0x2800007
#define MASK_VL1R_V  0xfff0707f
#define MATCH_VL2R_V 0x6805007
#define MASK_VL2R_V  0xfff0707f
#define MATCH_VL4R_V 0xe806007
#define MASK_VL4R_V  0xfff0707f
#define MATCH_VL8R_V 0x1e807007
#define MASK_VL8R_V  0xfff0707f
#define CSR_FFLAGS 0x1
#define CSR_FRM 0x2
#define CSR_FCSR 0x3
#define CSR_USTATUS 0x0
#define CSR_UIE 0x4
#define CSR_UTVEC 0x5
#define CSR_VSTART 0x8
#define CSR_VXSAT 0x9
#define CSR_VXRM 0xa
#define CSR_VCSR 0xf
#define CSR_USCRATCH 0x40
#define CSR_UEPC 0x41
#define CSR_UCAUSE 0x42
#define CSR_UTVAL 0x43
#define CSR_UIP 0x44
#define CSR_CYCLE 0xc00
#define CSR_TIME 0xc01
#define CSR_INSTRET 0xc02
#define CSR_HPMCOUNTER3 0xc03
#define CSR_HPMCOUNTER4 0xc04
#define CSR_HPMCOUNTER5 0xc05
#define CSR_HPMCOUNTER6 0xc06
#define CSR_HPMCOUNTER7 0xc07
#define CSR_HPMCOUNTER8 0xc08
#define CSR_HPMCOUNTER9 0xc09
#define CSR_HPMCOUNTER10 0xc0a
#define CSR_HPMCOUNTER11 0xc0b
#define CSR_HPMCOUNTER12 0xc0c
#define CSR_HPMCOUNTER13 0xc0d
#define CSR_HPMCOUNTER14 0xc0e
#define CSR_HPMCOUNTER15 0xc0f
#define CSR_HPMCOUNTER16 0xc10
#define CSR_HPMCOUNTER17 0xc11
#define CSR_HPMCOUNTER18 0xc12
#define CSR_HPMCOUNTER19 0xc13
#define CSR_HPMCOUNTER20 0xc14
#define CSR_HPMCOUNTER21 0xc15
#define CSR_HPMCOUNTER22 0xc16
#define CSR_HPMCOUNTER23 0xc17
#define CSR_HPMCOUNTER24 0xc18
#define CSR_HPMCOUNTER25 0xc19
#define CSR_HPMCOUNTER26 0xc1a
#define CSR_HPMCOUNTER27 0xc1b
#define CSR_HPMCOUNTER28 0xc1c
#define CSR_HPMCOUNTER29 0xc1d
#define CSR_HPMCOUNTER30 0xc1e
#define CSR_HPMCOUNTER31 0xc1f
#define CSR_VL 0xc20
#define CSR_VTYPE 0xc21
#define CSR_VLENB 0xc22
#define CSR_VME_SHAPE_S 0x400
#define CSR_VME_STRIDE_D 0x402
#define CSR_VME_STRIDE_S 0x403
#define CSR_MME_SHAPE_S1 0x404
#define CSR_MME_SHAPE_S2 0x405
#define CSR_MME_STRIDE_D 0x406
#define CSR_MME_STRIDE_S 0x407
#define CSR_CONV_FM_IN 0x408
#define CSR_CONV_CIN 0x409
#define CSR_CONV_FM_OUT 0x40a
#define CSR_CONV_COUT 0x40b
#define CSR_CONV_KERNEL_PARAMS2 0x40c
#define CSR_CONV_KERNEL_PARAMS1 0x40d
#define CSR_CONV_PADDING 0x40e
#define CSR_MME_DEQUANT_COEFF 0x410
#define CSR_MME_QUANT_COEFF 0x411
#define CSR_MME_SPARSEIDX_BASE 0x412
#define CSR_MME_SPARSEIDX_STRIDE 0x413
#define CSR_VME_DATA_TYPE 0x414
#define CSR_MME_DATA_TYPE 0x415
#define CSR_VME_RELU_THRESHHOLD 0x416
#define CSR_VME_FM_IN 0x417
#define CSR_VME_CIN 0x418
#define CSR_VME_FM_OUT 0x419
#define CSR_VME_COUT 0x41a
#define CSR_VME_KERNEL_PARAM1 0x41b
#define CSR_VME_FM_PADDING 0x41c
#define CSR_VME_KERNEL_PARAM2 0x41d
#define CSR_VME_MAX_MIN_IDX 0x41e
#define CSR_NCP_BUSY 0x41f
#define CSR_MTE_STRIDE_S 0x420
#define CSR_MTE_ICDEST 0x421
#define CSR_MTE_SHAPE 0x422
#define CSR_MTE_STRIDE_D 0x423
#define CSR_TID 0x424
#define CSR_TMISC 0x425
#define CSR_TCSR 0x426
#define CSR_MTE_DATA_TYPE 0x427
#define CSR_DMAE_DATA_TYPE 0x428
#define CSR_DMAE_SHAPE_1 0x429
#define CSR_DMAE_SHAPE_2 0x42a
#define CSR_DMAE_STRIDE_S1 0x42b
#define CSR_DMAE_STRIDE_S2 0x42c
#define CSR_DMAE_STRIDE_D1 0x42d
#define CSR_DMAE_STRIDE_D2 0x42e
#define CSR_DMAE_CTRL 0x42f
#define CSR_MDCM_CFG 0xfc1
#define CSR_MICM_CFG 0xfc0
#define CSR_MCACHE_CTL 0x7ca
#define CSR_MCCTLBEGINADDR 0x7cb
#define CSR_MCCTLCOMMAND 0x7cc
#define CSR_MCCTLDATA 0x7cd
#define CSR_SSTATUS 0x100
#define CSR_SEDELEG 0x102
#define CSR_SIDELEG 0x103
#define CSR_SIE 0x104
#define CSR_STVEC 0x105
#define CSR_SCOUNTEREN 0x106
#define CSR_SSCRATCH 0x140
#define CSR_SEPC 0x141
#define CSR_SCAUSE 0x142
#define CSR_STVAL 0x143
#define CSR_SIP 0x144
#define CSR_SATP 0x180
#define CSR_VSSTATUS 0x200
#define CSR_VSIE 0x204
#define CSR_VSTVEC 0x205
#define CSR_VSSCRATCH 0x240
#define CSR_VSEPC 0x241
#define CSR_VSCAUSE 0x242
#define CSR_VSTVAL 0x243
#define CSR_VSIP 0x244
#define CSR_VSATP 0x280
#define CSR_HSTATUS 0x600
#define CSR_HEDELEG 0x602
#define CSR_HIDELEG 0x603
#define CSR_HIE 0x604
#define CSR_HTIMEDELTA 0x605
#define CSR_HCOUNTEREN 0x606
#define CSR_HGEIE 0x607
#define CSR_HTVAL 0x643
#define CSR_HIP 0x644
#define CSR_HVIP 0x645
#define CSR_HTINST 0x64a
#define CSR_HGATP 0x680
#define CSR_HGEIP 0xe12
#define CSR_UTVT 0x7
#define CSR_UNXTI 0x45
#define CSR_UINTSTATUS 0x46
#define CSR_USCRATCHCSW 0x48
#define CSR_USCRATCHCSWL 0x49
#define CSR_STVT 0x107
#define CSR_SNXTI 0x145
#define CSR_SINTSTATUS 0x146
#define CSR_SSCRATCHCSW 0x148
#define CSR_SSCRATCHCSWL 0x149
#define CSR_MTVT 0x307
#define CSR_MNXTI 0x345
#define CSR_MINTSTATUS 0x346
#define CSR_MSCRATCHCSW 0x348
#define CSR_MSCRATCHCSWL 0x349
#define CSR_MSTATUS 0x300
#define CSR_MISA 0x301
#define CSR_MEDELEG 0x302
#define CSR_MIDELEG 0x303
#define CSR_MIE 0x304
#define CSR_MTVEC 0x305
#define CSR_MCOUNTEREN 0x306
#define CSR_MCOUNTINHIBIT 0x320
#define CSR_MSCRATCH 0x340
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_MTVAL 0x343
#define CSR_MIP 0x344
#define CSR_MTINST 0x34a
#define CSR_MTVAL2 0x34b
#define CSR_PMPCFG0 0x3a0
#define CSR_PMPCFG1 0x3a1
#define CSR_PMPCFG2 0x3a2
#define CSR_PMPCFG3 0x3a3
#define CSR_PMPADDR0 0x3b0
#define CSR_PMPADDR1 0x3b1
#define CSR_PMPADDR2 0x3b2
#define CSR_PMPADDR3 0x3b3
#define CSR_PMPADDR4 0x3b4
#define CSR_PMPADDR5 0x3b5
#define CSR_PMPADDR6 0x3b6
#define CSR_PMPADDR7 0x3b7
#define CSR_PMPADDR8 0x3b8
#define CSR_PMPADDR9 0x3b9
#define CSR_PMPADDR10 0x3ba
#define CSR_PMPADDR11 0x3bb
#define CSR_PMPADDR12 0x3bc
#define CSR_PMPADDR13 0x3bd
#define CSR_PMPADDR14 0x3be
#define CSR_PMPADDR15 0x3bf
#define CSR_TSELECT 0x7a0
#define CSR_TDATA1 0x7a1
#define CSR_TDATA2 0x7a2
#define CSR_TDATA3 0x7a3
#define CSR_DCSR 0x7b0
#define CSR_DPC 0x7b1
#define CSR_DSCRATCH0 0x7b2
#define CSR_DSCRATCH1 0x7b3
#define CSR_MHSP_CTL 0x7c6
#define CSR_MSP_BOUND 0x7c7
#define CSR_MSP_BASE 0x7c8
#define CSR_MCYCLE 0xb00
#define CSR_MINSTRET 0xb02
#define CSR_MHPMCOUNTER3 0xb03
#define CSR_MHPMCOUNTER4 0xb04
#define CSR_MHPMCOUNTER5 0xb05
#define CSR_MHPMCOUNTER6 0xb06
#define CSR_MHPMCOUNTER7 0xb07
#define CSR_MHPMCOUNTER8 0xb08
#define CSR_MHPMCOUNTER9 0xb09
#define CSR_MHPMCOUNTER10 0xb0a
#define CSR_MHPMCOUNTER11 0xb0b
#define CSR_MHPMCOUNTER12 0xb0c
#define CSR_MHPMCOUNTER13 0xb0d
#define CSR_MHPMCOUNTER14 0xb0e
#define CSR_MHPMCOUNTER15 0xb0f
#define CSR_MHPMCOUNTER16 0xb10
#define CSR_MHPMCOUNTER17 0xb11
#define CSR_MHPMCOUNTER18 0xb12
#define CSR_MHPMCOUNTER19 0xb13
#define CSR_MHPMCOUNTER20 0xb14
#define CSR_MHPMCOUNTER21 0xb15
#define CSR_MHPMCOUNTER22 0xb16
#define CSR_MHPMCOUNTER23 0xb17
#define CSR_MHPMCOUNTER24 0xb18
#define CSR_MHPMCOUNTER25 0xb19
#define CSR_MHPMCOUNTER26 0xb1a
#define CSR_MHPMCOUNTER27 0xb1b
#define CSR_MHPMCOUNTER28 0xb1c
#define CSR_MHPMCOUNTER29 0xb1d
#define CSR_MHPMCOUNTER30 0xb1e
#define CSR_MHPMCOUNTER31 0xb1f
#define CSR_MHPMEVENT3 0x323
#define CSR_MHPMEVENT4 0x324
#define CSR_MHPMEVENT5 0x325
#define CSR_MHPMEVENT6 0x326
#define CSR_MHPMEVENT7 0x327
#define CSR_MHPMEVENT8 0x328
#define CSR_MHPMEVENT9 0x329
#define CSR_MHPMEVENT10 0x32a
#define CSR_MHPMEVENT11 0x32b
#define CSR_MHPMEVENT12 0x32c
#define CSR_MHPMEVENT13 0x32d
#define CSR_MHPMEVENT14 0x32e
#define CSR_MHPMEVENT15 0x32f
#define CSR_MHPMEVENT16 0x330
#define CSR_MHPMEVENT17 0x331
#define CSR_MHPMEVENT18 0x332
#define CSR_MHPMEVENT19 0x333
#define CSR_MHPMEVENT20 0x334
#define CSR_MHPMEVENT21 0x335
#define CSR_MHPMEVENT22 0x336
#define CSR_MHPMEVENT23 0x337
#define CSR_MHPMEVENT24 0x338
#define CSR_MHPMEVENT25 0x339
#define CSR_MHPMEVENT26 0x33a
#define CSR_MHPMEVENT27 0x33b
#define CSR_MHPMEVENT28 0x33c
#define CSR_MHPMEVENT29 0x33d
#define CSR_MHPMEVENT30 0x33e
#define CSR_MHPMEVENT31 0x33f
#define CSR_MVENDORID 0xf11
#define CSR_MARCHID 0xf12
#define CSR_MIMPID 0xf13
#define CSR_MHARTID 0xf14
#define CSR_HTIMEDELTAH 0x615
#define CSR_CYCLEH 0xc80
#define CSR_TIMEH 0xc81
#define CSR_INSTRETH 0xc82
#define CSR_HPMCOUNTER3H 0xc83
#define CSR_HPMCOUNTER4H 0xc84
#define CSR_HPMCOUNTER5H 0xc85
#define CSR_HPMCOUNTER6H 0xc86
#define CSR_HPMCOUNTER7H 0xc87
#define CSR_HPMCOUNTER8H 0xc88
#define CSR_HPMCOUNTER9H 0xc89
#define CSR_HPMCOUNTER10H 0xc8a
#define CSR_HPMCOUNTER11H 0xc8b
#define CSR_HPMCOUNTER12H 0xc8c
#define CSR_HPMCOUNTER13H 0xc8d
#define CSR_HPMCOUNTER14H 0xc8e
#define CSR_HPMCOUNTER15H 0xc8f
#define CSR_HPMCOUNTER16H 0xc90
#define CSR_HPMCOUNTER17H 0xc91
#define CSR_HPMCOUNTER18H 0xc92
#define CSR_HPMCOUNTER19H 0xc93
#define CSR_HPMCOUNTER20H 0xc94
#define CSR_HPMCOUNTER21H 0xc95
#define CSR_HPMCOUNTER22H 0xc96
#define CSR_HPMCOUNTER23H 0xc97
#define CSR_HPMCOUNTER24H 0xc98
#define CSR_HPMCOUNTER25H 0xc99
#define CSR_HPMCOUNTER26H 0xc9a
#define CSR_HPMCOUNTER27H 0xc9b
#define CSR_HPMCOUNTER28H 0xc9c
#define CSR_HPMCOUNTER29H 0xc9d
#define CSR_HPMCOUNTER30H 0xc9e
#define CSR_HPMCOUNTER31H 0xc9f
#define CSR_MSTATUSH 0x310
#define CSR_MCYCLEH 0xb80
#define CSR_MINSTRETH 0xb82
#define CSR_MHPMCOUNTER3H 0xb83
#define CSR_MHPMCOUNTER4H 0xb84
#define CSR_MHPMCOUNTER5H 0xb85
#define CSR_MHPMCOUNTER6H 0xb86
#define CSR_MHPMCOUNTER7H 0xb87
#define CSR_MHPMCOUNTER8H 0xb88
#define CSR_MHPMCOUNTER9H 0xb89
#define CSR_MHPMCOUNTER10H 0xb8a
#define CSR_MHPMCOUNTER11H 0xb8b
#define CSR_MHPMCOUNTER12H 0xb8c
#define CSR_MHPMCOUNTER13H 0xb8d
#define CSR_MHPMCOUNTER14H 0xb8e
#define CSR_MHPMCOUNTER15H 0xb8f
#define CSR_MHPMCOUNTER16H 0xb90
#define CSR_MHPMCOUNTER17H 0xb91
#define CSR_MHPMCOUNTER18H 0xb92
#define CSR_MHPMCOUNTER19H 0xb93
#define CSR_MHPMCOUNTER20H 0xb94
#define CSR_MHPMCOUNTER21H 0xb95
#define CSR_MHPMCOUNTER22H 0xb96
#define CSR_MHPMCOUNTER23H 0xb97
#define CSR_MHPMCOUNTER24H 0xb98
#define CSR_MHPMCOUNTER25H 0xb99
#define CSR_MHPMCOUNTER26H 0xb9a
#define CSR_MHPMCOUNTER27H 0xb9b
#define CSR_MHPMCOUNTER28H 0xb9c
#define CSR_MHPMCOUNTER29H 0xb9d
#define CSR_MHPMCOUNTER30H 0xb9e
#define CSR_MHPMCOUNTER31H 0xb9f
#define CAUSE_MISALIGNED_FETCH 0x0
#define CAUSE_FETCH_ACCESS 0x1
#define CAUSE_ILLEGAL_INSTRUCTION 0x2
#define CAUSE_BREAKPOINT 0x3
#define CAUSE_MISALIGNED_LOAD 0x4
#define CAUSE_LOAD_ACCESS 0x5
#define CAUSE_MISALIGNED_STORE 0x6
#define CAUSE_STORE_ACCESS 0x7
#define CAUSE_USER_ECALL 0x8
#define CAUSE_SUPERVISOR_ECALL 0x9
#define CAUSE_VIRTUAL_SUPERVISOR_ECALL 0xa
#define CAUSE_MACHINE_ECALL 0xb
#define CAUSE_FETCH_PAGE_FAULT 0xc
#define CAUSE_LOAD_PAGE_FAULT 0xd
#define CAUSE_STORE_PAGE_FAULT 0xf
#define CAUSE_FETCH_GUEST_PAGE_FAULT 0x14
#define CAUSE_LOAD_GUEST_PAGE_FAULT 0x15
#define CAUSE_VIRTUAL_INSTRUCTION 0x16
#define CAUSE_STORE_GUEST_PAGE_FAULT 0x17
#define CAUSE_STACK_OVERFLOW_EXCEPTION 0x20
#define CAUSE_STACK_UNDERFLOW_EXCEPTION 0x21
#define CAUSE_NCP_ILLEGAL_ENCODING 0x28
#define CAUSE_NCP_VILL_INVALID_INST 0x29
#define CAUSE_NCP_WR_VL_VTYPE 0x2a
#define CAUSE_NCP_RW_ILLEGAL_CSR 0x2b
#define CAUSE_NCP_RVV_MISALIGNED_BASE 0x2d
#define CAUSE_NCP_RVV_MISALIGNED_OFFSET 0x2e
#define CAUSE_NCP_RVV_ACCESS 0x2f
#define CAUSE_NCP_RVV_INVALID_SAME_RDRS 0x30
#define CAUSE_NCP_CUST_MISALIGNED_BASE 0x32
#define CAUSE_NCP_CUST_INVALID_PARAM 0x33
#define CAUSE_NCP_CUST_MISALIGNED_STRIDE 0x34
#define CAUSE_NCP_CUST_ACCESS 0x35
#define CAUSE_TCP_ILLEGAL_ENCODING 0x38
#define CAUSE_TCP_ICMOV_INVALID_CORE 0x39
#define CAUSE_TCP_ACCESS_START 0x3a
#define CAUSE_TCP_ACCESS_START_ICMOV 0x3b
#define CAUSE_TCP_ACCESS_END_L1 0x3c
#define CAUSE_TCP_ACCESS_END_LLB 0x3d
#define CAUSE_TCP_INVALID_PARAM 0x3e
#define CAUSE_TCP_RW_ILLEGAL_CSR 0x3f
#endif
#ifdef DECLARE_INSN
DECLARE_INSN(slli_rv32, MATCH_SLLI_RV32, MASK_SLLI_RV32)
DECLARE_INSN(srli_rv32, MATCH_SRLI_RV32, MASK_SRLI_RV32)
DECLARE_INSN(srai_rv32, MATCH_SRAI_RV32, MASK_SRAI_RV32)
DECLARE_INSN(frflags, MATCH_FRFLAGS, MASK_FRFLAGS)
DECLARE_INSN(fsflags, MATCH_FSFLAGS, MASK_FSFLAGS)
DECLARE_INSN(fsflagsi, MATCH_FSFLAGSI, MASK_FSFLAGSI)
DECLARE_INSN(frrm, MATCH_FRRM, MASK_FRRM)
DECLARE_INSN(fsrm, MATCH_FSRM, MASK_FSRM)
DECLARE_INSN(fsrmi, MATCH_FSRMI, MASK_FSRMI)
DECLARE_INSN(fscsr, MATCH_FSCSR, MASK_FSCSR)
DECLARE_INSN(frcsr, MATCH_FRCSR, MASK_FRCSR)
DECLARE_INSN(rdcycle, MATCH_RDCYCLE, MASK_RDCYCLE)
DECLARE_INSN(rdtime, MATCH_RDTIME, MASK_RDTIME)
DECLARE_INSN(rdinstret, MATCH_RDINSTRET, MASK_RDINSTRET)
DECLARE_INSN(rdcycleh, MATCH_RDCYCLEH, MASK_RDCYCLEH)
DECLARE_INSN(rdtimeh, MATCH_RDTIMEH, MASK_RDTIMEH)
DECLARE_INSN(rdinstreth, MATCH_RDINSTRETH, MASK_RDINSTRETH)
DECLARE_INSN(scall, MATCH_SCALL, MASK_SCALL)
DECLARE_INSN(sbreak, MATCH_SBREAK, MASK_SBREAK)
DECLARE_INSN(fmv_x_s, MATCH_FMV_X_S, MASK_FMV_X_S)
DECLARE_INSN(fmv_s_x, MATCH_FMV_S_X, MASK_FMV_S_X)
DECLARE_INSN(fence_tso, MATCH_FENCE_TSO, MASK_FENCE_TSO)
DECLARE_INSN(pause, MATCH_PAUSE, MASK_PAUSE)
DECLARE_INSN(beq, MATCH_BEQ, MASK_BEQ)
DECLARE_INSN(bne, MATCH_BNE, MASK_BNE)
DECLARE_INSN(blt, MATCH_BLT, MASK_BLT)
DECLARE_INSN(bge, MATCH_BGE, MASK_BGE)
DECLARE_INSN(bltu, MATCH_BLTU, MASK_BLTU)
DECLARE_INSN(bgeu, MATCH_BGEU, MASK_BGEU)
DECLARE_INSN(jalr, MATCH_JALR, MASK_JALR)
DECLARE_INSN(jal, MATCH_JAL, MASK_JAL)
DECLARE_INSN(lui, MATCH_LUI, MASK_LUI)
DECLARE_INSN(auipc, MATCH_AUIPC, MASK_AUIPC)
DECLARE_INSN(addi, MATCH_ADDI, MASK_ADDI)
DECLARE_INSN(slli, MATCH_SLLI, MASK_SLLI)
DECLARE_INSN(slti, MATCH_SLTI, MASK_SLTI)
DECLARE_INSN(sltiu, MATCH_SLTIU, MASK_SLTIU)
DECLARE_INSN(xori, MATCH_XORI, MASK_XORI)
DECLARE_INSN(srli, MATCH_SRLI, MASK_SRLI)
DECLARE_INSN(srai, MATCH_SRAI, MASK_SRAI)
DECLARE_INSN(ori, MATCH_ORI, MASK_ORI)
DECLARE_INSN(andi, MATCH_ANDI, MASK_ANDI)
DECLARE_INSN(add, MATCH_ADD, MASK_ADD)
DECLARE_INSN(sub, MATCH_SUB, MASK_SUB)
DECLARE_INSN(sll, MATCH_SLL, MASK_SLL)
DECLARE_INSN(slt, MATCH_SLT, MASK_SLT)
DECLARE_INSN(sltu, MATCH_SLTU, MASK_SLTU)
DECLARE_INSN(xor, MATCH_XOR, MASK_XOR)
DECLARE_INSN(srl, MATCH_SRL, MASK_SRL)
DECLARE_INSN(sra, MATCH_SRA, MASK_SRA)
DECLARE_INSN(or, MATCH_OR, MASK_OR)
DECLARE_INSN(and, MATCH_AND, MASK_AND)
DECLARE_INSN(lb, MATCH_LB, MASK_LB)
DECLARE_INSN(lh, MATCH_LH, MASK_LH)
DECLARE_INSN(lw, MATCH_LW, MASK_LW)
DECLARE_INSN(lbu, MATCH_LBU, MASK_LBU)
DECLARE_INSN(lhu, MATCH_LHU, MASK_LHU)
DECLARE_INSN(sb, MATCH_SB, MASK_SB)
DECLARE_INSN(sh, MATCH_SH, MASK_SH)
DECLARE_INSN(sw, MATCH_SW, MASK_SW)
DECLARE_INSN(fence, MATCH_FENCE, MASK_FENCE)
DECLARE_INSN(fence_i, MATCH_FENCE_I, MASK_FENCE_I)
DECLARE_INSN(addiw, MATCH_ADDIW, MASK_ADDIW)
DECLARE_INSN(slliw, MATCH_SLLIW, MASK_SLLIW)
DECLARE_INSN(srliw, MATCH_SRLIW, MASK_SRLIW)
DECLARE_INSN(sraiw, MATCH_SRAIW, MASK_SRAIW)
DECLARE_INSN(addw, MATCH_ADDW, MASK_ADDW)
DECLARE_INSN(subw, MATCH_SUBW, MASK_SUBW)
DECLARE_INSN(sllw, MATCH_SLLW, MASK_SLLW)
DECLARE_INSN(srlw, MATCH_SRLW, MASK_SRLW)
DECLARE_INSN(sraw, MATCH_SRAW, MASK_SRAW)
DECLARE_INSN(ld, MATCH_LD, MASK_LD)
DECLARE_INSN(lwu, MATCH_LWU, MASK_LWU)
DECLARE_INSN(sd, MATCH_SD, MASK_SD)
DECLARE_INSN(mul, MATCH_MUL, MASK_MUL)
DECLARE_INSN(mulh, MATCH_MULH, MASK_MULH)
DECLARE_INSN(mulhsu, MATCH_MULHSU, MASK_MULHSU)
DECLARE_INSN(mulhu, MATCH_MULHU, MASK_MULHU)
DECLARE_INSN(div, MATCH_DIV, MASK_DIV)
DECLARE_INSN(divu, MATCH_DIVU, MASK_DIVU)
DECLARE_INSN(rem, MATCH_REM, MASK_REM)
DECLARE_INSN(remu, MATCH_REMU, MASK_REMU)
DECLARE_INSN(mulw, MATCH_MULW, MASK_MULW)
DECLARE_INSN(divw, MATCH_DIVW, MASK_DIVW)
DECLARE_INSN(divuw, MATCH_DIVUW, MASK_DIVUW)
DECLARE_INSN(remw, MATCH_REMW, MASK_REMW)
DECLARE_INSN(remuw, MATCH_REMUW, MASK_REMUW)
DECLARE_INSN(amoadd_w, MATCH_AMOADD_W, MASK_AMOADD_W)
DECLARE_INSN(amoxor_w, MATCH_AMOXOR_W, MASK_AMOXOR_W)
DECLARE_INSN(amoor_w, MATCH_AMOOR_W, MASK_AMOOR_W)
DECLARE_INSN(amoand_w, MATCH_AMOAND_W, MASK_AMOAND_W)
DECLARE_INSN(amomin_w, MATCH_AMOMIN_W, MASK_AMOMIN_W)
DECLARE_INSN(amomax_w, MATCH_AMOMAX_W, MASK_AMOMAX_W)
DECLARE_INSN(amominu_w, MATCH_AMOMINU_W, MASK_AMOMINU_W)
DECLARE_INSN(amomaxu_w, MATCH_AMOMAXU_W, MASK_AMOMAXU_W)
DECLARE_INSN(amoswap_w, MATCH_AMOSWAP_W, MASK_AMOSWAP_W)
DECLARE_INSN(lr_w, MATCH_LR_W, MASK_LR_W)
DECLARE_INSN(sc_w, MATCH_SC_W, MASK_SC_W)
DECLARE_INSN(amoadd_d, MATCH_AMOADD_D, MASK_AMOADD_D)
DECLARE_INSN(amoxor_d, MATCH_AMOXOR_D, MASK_AMOXOR_D)
DECLARE_INSN(amoor_d, MATCH_AMOOR_D, MASK_AMOOR_D)
DECLARE_INSN(amoand_d, MATCH_AMOAND_D, MASK_AMOAND_D)
DECLARE_INSN(amomin_d, MATCH_AMOMIN_D, MASK_AMOMIN_D)
DECLARE_INSN(amomax_d, MATCH_AMOMAX_D, MASK_AMOMAX_D)
DECLARE_INSN(amominu_d, MATCH_AMOMINU_D, MASK_AMOMINU_D)
DECLARE_INSN(amomaxu_d, MATCH_AMOMAXU_D, MASK_AMOMAXU_D)
DECLARE_INSN(amoswap_d, MATCH_AMOSWAP_D, MASK_AMOSWAP_D)
DECLARE_INSN(lr_d, MATCH_LR_D, MASK_LR_D)
DECLARE_INSN(sc_d, MATCH_SC_D, MASK_SC_D)
DECLARE_INSN(hfence_vvma, MATCH_HFENCE_VVMA, MASK_HFENCE_VVMA)
DECLARE_INSN(hfence_gvma, MATCH_HFENCE_GVMA, MASK_HFENCE_GVMA)
DECLARE_INSN(hlv_b, MATCH_HLV_B, MASK_HLV_B)
DECLARE_INSN(hlv_bu, MATCH_HLV_BU, MASK_HLV_BU)
DECLARE_INSN(hlv_h, MATCH_HLV_H, MASK_HLV_H)
DECLARE_INSN(hlv_hu, MATCH_HLV_HU, MASK_HLV_HU)
DECLARE_INSN(hlvx_hu, MATCH_HLVX_HU, MASK_HLVX_HU)
DECLARE_INSN(hlv_w, MATCH_HLV_W, MASK_HLV_W)
DECLARE_INSN(hlvx_wu, MATCH_HLVX_WU, MASK_HLVX_WU)
DECLARE_INSN(hsv_b, MATCH_HSV_B, MASK_HSV_B)
DECLARE_INSN(hsv_h, MATCH_HSV_H, MASK_HSV_H)
DECLARE_INSN(hsv_w, MATCH_HSV_W, MASK_HSV_W)
DECLARE_INSN(hlv_wu, MATCH_HLV_WU, MASK_HLV_WU)
DECLARE_INSN(hlv_d, MATCH_HLV_D, MASK_HLV_D)
DECLARE_INSN(hsv_d, MATCH_HSV_D, MASK_HSV_D)
DECLARE_INSN(fadd_s, MATCH_FADD_S, MASK_FADD_S)
DECLARE_INSN(fsub_s, MATCH_FSUB_S, MASK_FSUB_S)
DECLARE_INSN(fmul_s, MATCH_FMUL_S, MASK_FMUL_S)
DECLARE_INSN(fdiv_s, MATCH_FDIV_S, MASK_FDIV_S)
DECLARE_INSN(fsgnj_s, MATCH_FSGNJ_S, MASK_FSGNJ_S)
DECLARE_INSN(fsgnjn_s, MATCH_FSGNJN_S, MASK_FSGNJN_S)
DECLARE_INSN(fsgnjx_s, MATCH_FSGNJX_S, MASK_FSGNJX_S)
DECLARE_INSN(fmin_s, MATCH_FMIN_S, MASK_FMIN_S)
DECLARE_INSN(fmax_s, MATCH_FMAX_S, MASK_FMAX_S)
DECLARE_INSN(fsqrt_s, MATCH_FSQRT_S, MASK_FSQRT_S)
DECLARE_INSN(fle_s, MATCH_FLE_S, MASK_FLE_S)
DECLARE_INSN(flt_s, MATCH_FLT_S, MASK_FLT_S)
DECLARE_INSN(feq_s, MATCH_FEQ_S, MASK_FEQ_S)
DECLARE_INSN(fcvt_w_s, MATCH_FCVT_W_S, MASK_FCVT_W_S)
DECLARE_INSN(fcvt_wu_s, MATCH_FCVT_WU_S, MASK_FCVT_WU_S)
DECLARE_INSN(fmv_x_w, MATCH_FMV_X_W, MASK_FMV_X_W)
DECLARE_INSN(fclass_s, MATCH_FCLASS_S, MASK_FCLASS_S)
DECLARE_INSN(fcvt_s_w, MATCH_FCVT_S_W, MASK_FCVT_S_W)
DECLARE_INSN(fcvt_s_wu, MATCH_FCVT_S_WU, MASK_FCVT_S_WU)
DECLARE_INSN(fmv_w_x, MATCH_FMV_W_X, MASK_FMV_W_X)
DECLARE_INSN(flw, MATCH_FLW, MASK_FLW)
DECLARE_INSN(fsw, MATCH_FSW, MASK_FSW)
DECLARE_INSN(fmadd_s, MATCH_FMADD_S, MASK_FMADD_S)
DECLARE_INSN(fmsub_s, MATCH_FMSUB_S, MASK_FMSUB_S)
DECLARE_INSN(fnmsub_s, MATCH_FNMSUB_S, MASK_FNMSUB_S)
DECLARE_INSN(fnmadd_s, MATCH_FNMADD_S, MASK_FNMADD_S)
DECLARE_INSN(fcvt_l_s, MATCH_FCVT_L_S, MASK_FCVT_L_S)
DECLARE_INSN(fcvt_lu_s, MATCH_FCVT_LU_S, MASK_FCVT_LU_S)
DECLARE_INSN(fcvt_s_l, MATCH_FCVT_S_L, MASK_FCVT_S_L)
DECLARE_INSN(fcvt_s_lu, MATCH_FCVT_S_LU, MASK_FCVT_S_LU)
DECLARE_INSN(fadd_d, MATCH_FADD_D, MASK_FADD_D)
DECLARE_INSN(fsub_d, MATCH_FSUB_D, MASK_FSUB_D)
DECLARE_INSN(fmul_d, MATCH_FMUL_D, MASK_FMUL_D)
DECLARE_INSN(fdiv_d, MATCH_FDIV_D, MASK_FDIV_D)
DECLARE_INSN(fsgnj_d, MATCH_FSGNJ_D, MASK_FSGNJ_D)
DECLARE_INSN(fsgnjn_d, MATCH_FSGNJN_D, MASK_FSGNJN_D)
DECLARE_INSN(fsgnjx_d, MATCH_FSGNJX_D, MASK_FSGNJX_D)
DECLARE_INSN(fmin_d, MATCH_FMIN_D, MASK_FMIN_D)
DECLARE_INSN(fmax_d, MATCH_FMAX_D, MASK_FMAX_D)
DECLARE_INSN(fcvt_s_d, MATCH_FCVT_S_D, MASK_FCVT_S_D)
DECLARE_INSN(fcvt_d_s, MATCH_FCVT_D_S, MASK_FCVT_D_S)
DECLARE_INSN(fsqrt_d, MATCH_FSQRT_D, MASK_FSQRT_D)
DECLARE_INSN(fle_d, MATCH_FLE_D, MASK_FLE_D)
DECLARE_INSN(flt_d, MATCH_FLT_D, MASK_FLT_D)
DECLARE_INSN(feq_d, MATCH_FEQ_D, MASK_FEQ_D)
DECLARE_INSN(fcvt_w_d, MATCH_FCVT_W_D, MASK_FCVT_W_D)
DECLARE_INSN(fcvt_wu_d, MATCH_FCVT_WU_D, MASK_FCVT_WU_D)
DECLARE_INSN(fclass_d, MATCH_FCLASS_D, MASK_FCLASS_D)
DECLARE_INSN(fcvt_d_w, MATCH_FCVT_D_W, MASK_FCVT_D_W)
DECLARE_INSN(fcvt_d_wu, MATCH_FCVT_D_WU, MASK_FCVT_D_WU)
DECLARE_INSN(fld, MATCH_FLD, MASK_FLD)
DECLARE_INSN(fsd, MATCH_FSD, MASK_FSD)
DECLARE_INSN(fmadd_d, MATCH_FMADD_D, MASK_FMADD_D)
DECLARE_INSN(fmsub_d, MATCH_FMSUB_D, MASK_FMSUB_D)
DECLARE_INSN(fnmsub_d, MATCH_FNMSUB_D, MASK_FNMSUB_D)
DECLARE_INSN(fnmadd_d, MATCH_FNMADD_D, MASK_FNMADD_D)
DECLARE_INSN(fcvt_l_d, MATCH_FCVT_L_D, MASK_FCVT_L_D)
DECLARE_INSN(fcvt_lu_d, MATCH_FCVT_LU_D, MASK_FCVT_LU_D)
DECLARE_INSN(fmv_x_d, MATCH_FMV_X_D, MASK_FMV_X_D)
DECLARE_INSN(fcvt_d_l, MATCH_FCVT_D_L, MASK_FCVT_D_L)
DECLARE_INSN(fcvt_d_lu, MATCH_FCVT_D_LU, MASK_FCVT_D_LU)
DECLARE_INSN(fmv_d_x, MATCH_FMV_D_X, MASK_FMV_D_X)
DECLARE_INSN(fadd_q, MATCH_FADD_Q, MASK_FADD_Q)
DECLARE_INSN(fsub_q, MATCH_FSUB_Q, MASK_FSUB_Q)
DECLARE_INSN(fmul_q, MATCH_FMUL_Q, MASK_FMUL_Q)
DECLARE_INSN(fdiv_q, MATCH_FDIV_Q, MASK_FDIV_Q)
DECLARE_INSN(fsgnj_q, MATCH_FSGNJ_Q, MASK_FSGNJ_Q)
DECLARE_INSN(fsgnjn_q, MATCH_FSGNJN_Q, MASK_FSGNJN_Q)
DECLARE_INSN(fsgnjx_q, MATCH_FSGNJX_Q, MASK_FSGNJX_Q)
DECLARE_INSN(fmin_q, MATCH_FMIN_Q, MASK_FMIN_Q)
DECLARE_INSN(fmax_q, MATCH_FMAX_Q, MASK_FMAX_Q)
DECLARE_INSN(fcvt_s_q, MATCH_FCVT_S_Q, MASK_FCVT_S_Q)
DECLARE_INSN(fcvt_q_s, MATCH_FCVT_Q_S, MASK_FCVT_Q_S)
DECLARE_INSN(fcvt_d_q, MATCH_FCVT_D_Q, MASK_FCVT_D_Q)
DECLARE_INSN(fcvt_q_d, MATCH_FCVT_Q_D, MASK_FCVT_Q_D)
DECLARE_INSN(fsqrt_q, MATCH_FSQRT_Q, MASK_FSQRT_Q)
DECLARE_INSN(fle_q, MATCH_FLE_Q, MASK_FLE_Q)
DECLARE_INSN(flt_q, MATCH_FLT_Q, MASK_FLT_Q)
DECLARE_INSN(feq_q, MATCH_FEQ_Q, MASK_FEQ_Q)
DECLARE_INSN(fcvt_w_q, MATCH_FCVT_W_Q, MASK_FCVT_W_Q)
DECLARE_INSN(fcvt_wu_q, MATCH_FCVT_WU_Q, MASK_FCVT_WU_Q)
DECLARE_INSN(fclass_q, MATCH_FCLASS_Q, MASK_FCLASS_Q)
DECLARE_INSN(fcvt_q_w, MATCH_FCVT_Q_W, MASK_FCVT_Q_W)
DECLARE_INSN(fcvt_q_wu, MATCH_FCVT_Q_WU, MASK_FCVT_Q_WU)
DECLARE_INSN(flq, MATCH_FLQ, MASK_FLQ)
DECLARE_INSN(fsq, MATCH_FSQ, MASK_FSQ)
DECLARE_INSN(fmadd_q, MATCH_FMADD_Q, MASK_FMADD_Q)
DECLARE_INSN(fmsub_q, MATCH_FMSUB_Q, MASK_FMSUB_Q)
DECLARE_INSN(fnmsub_q, MATCH_FNMSUB_Q, MASK_FNMSUB_Q)
DECLARE_INSN(fnmadd_q, MATCH_FNMADD_Q, MASK_FNMADD_Q)
DECLARE_INSN(fcvt_l_q, MATCH_FCVT_L_Q, MASK_FCVT_L_Q)
DECLARE_INSN(fcvt_lu_q, MATCH_FCVT_LU_Q, MASK_FCVT_LU_Q)
DECLARE_INSN(fcvt_q_l, MATCH_FCVT_Q_L, MASK_FCVT_Q_L)
DECLARE_INSN(fcvt_q_lu, MATCH_FCVT_Q_LU, MASK_FCVT_Q_LU)
DECLARE_INSN(andn, MATCH_ANDN, MASK_ANDN)
DECLARE_INSN(orn, MATCH_ORN, MASK_ORN)
DECLARE_INSN(xnor, MATCH_XNOR, MASK_XNOR)
DECLARE_INSN(slo, MATCH_SLO, MASK_SLO)
DECLARE_INSN(sro, MATCH_SRO, MASK_SRO)
DECLARE_INSN(rol, MATCH_ROL, MASK_ROL)
DECLARE_INSN(ror, MATCH_ROR, MASK_ROR)
DECLARE_INSN(sbclr, MATCH_SBCLR, MASK_SBCLR)
DECLARE_INSN(sbset, MATCH_SBSET, MASK_SBSET)
DECLARE_INSN(sbinv, MATCH_SBINV, MASK_SBINV)
DECLARE_INSN(sbext, MATCH_SBEXT, MASK_SBEXT)
DECLARE_INSN(gorc, MATCH_GORC, MASK_GORC)
DECLARE_INSN(grev, MATCH_GREV, MASK_GREV)
DECLARE_INSN(sloi, MATCH_SLOI, MASK_SLOI)
DECLARE_INSN(sroi, MATCH_SROI, MASK_SROI)
DECLARE_INSN(rori, MATCH_RORI, MASK_RORI)
DECLARE_INSN(sbclri, MATCH_SBCLRI, MASK_SBCLRI)
DECLARE_INSN(sbseti, MATCH_SBSETI, MASK_SBSETI)
DECLARE_INSN(sbinvi, MATCH_SBINVI, MASK_SBINVI)
DECLARE_INSN(sbexti, MATCH_SBEXTI, MASK_SBEXTI)
DECLARE_INSN(gorci, MATCH_GORCI, MASK_GORCI)
DECLARE_INSN(grevi, MATCH_GREVI, MASK_GREVI)
DECLARE_INSN(cmix, MATCH_CMIX, MASK_CMIX)
DECLARE_INSN(cmov, MATCH_CMOV, MASK_CMOV)
DECLARE_INSN(fsl, MATCH_FSL, MASK_FSL)
DECLARE_INSN(fsr, MATCH_FSR, MASK_FSR)
DECLARE_INSN(fsri, MATCH_FSRI, MASK_FSRI)
DECLARE_INSN(clz, MATCH_CLZ, MASK_CLZ)
DECLARE_INSN(ctz, MATCH_CTZ, MASK_CTZ)
DECLARE_INSN(pcnt, MATCH_PCNT, MASK_PCNT)
DECLARE_INSN(sext_b, MATCH_SEXT_B, MASK_SEXT_B)
DECLARE_INSN(sext_h, MATCH_SEXT_H, MASK_SEXT_H)
DECLARE_INSN(crc32_b, MATCH_CRC32_B, MASK_CRC32_B)
DECLARE_INSN(crc32_h, MATCH_CRC32_H, MASK_CRC32_H)
DECLARE_INSN(crc32_w, MATCH_CRC32_W, MASK_CRC32_W)
DECLARE_INSN(crc32c_b, MATCH_CRC32C_B, MASK_CRC32C_B)
DECLARE_INSN(crc32c_h, MATCH_CRC32C_H, MASK_CRC32C_H)
DECLARE_INSN(crc32c_w, MATCH_CRC32C_W, MASK_CRC32C_W)
DECLARE_INSN(sh1add, MATCH_SH1ADD, MASK_SH1ADD)
DECLARE_INSN(sh2add, MATCH_SH2ADD, MASK_SH2ADD)
DECLARE_INSN(sh3add, MATCH_SH3ADD, MASK_SH3ADD)
DECLARE_INSN(clmul, MATCH_CLMUL, MASK_CLMUL)
DECLARE_INSN(clmulr, MATCH_CLMULR, MASK_CLMULR)
DECLARE_INSN(clmulh, MATCH_CLMULH, MASK_CLMULH)
DECLARE_INSN(min, MATCH_MIN, MASK_MIN)
DECLARE_INSN(minu, MATCH_MINU, MASK_MINU)
DECLARE_INSN(max, MATCH_MAX, MASK_MAX)
DECLARE_INSN(maxu, MATCH_MAXU, MASK_MAXU)
DECLARE_INSN(shfl, MATCH_SHFL, MASK_SHFL)
DECLARE_INSN(unshfl, MATCH_UNSHFL, MASK_UNSHFL)
DECLARE_INSN(bext, MATCH_BEXT, MASK_BEXT)
DECLARE_INSN(bdep, MATCH_BDEP, MASK_BDEP)
DECLARE_INSN(pack, MATCH_PACK, MASK_PACK)
DECLARE_INSN(packu, MATCH_PACKU, MASK_PACKU)
DECLARE_INSN(packh, MATCH_PACKH, MASK_PACKH)
DECLARE_INSN(bfp, MATCH_BFP, MASK_BFP)
DECLARE_INSN(shfli, MATCH_SHFLI, MASK_SHFLI)
DECLARE_INSN(unshfli, MATCH_UNSHFLI, MASK_UNSHFLI)
DECLARE_INSN(bmatflip, MATCH_BMATFLIP, MASK_BMATFLIP)
DECLARE_INSN(crc32_d, MATCH_CRC32_D, MASK_CRC32_D)
DECLARE_INSN(crc32c_d, MATCH_CRC32C_D, MASK_CRC32C_D)
DECLARE_INSN(bmator, MATCH_BMATOR, MASK_BMATOR)
DECLARE_INSN(bmatxor, MATCH_BMATXOR, MASK_BMATXOR)
DECLARE_INSN(addiwu, MATCH_ADDIWU, MASK_ADDIWU)
DECLARE_INSN(slliu_w, MATCH_SLLIU_W, MASK_SLLIU_W)
DECLARE_INSN(addwu, MATCH_ADDWU, MASK_ADDWU)
DECLARE_INSN(subwu, MATCH_SUBWU, MASK_SUBWU)
DECLARE_INSN(addu_w, MATCH_ADDU_W, MASK_ADDU_W)
DECLARE_INSN(slow, MATCH_SLOW, MASK_SLOW)
DECLARE_INSN(srow, MATCH_SROW, MASK_SROW)
DECLARE_INSN(rolw, MATCH_ROLW, MASK_ROLW)
DECLARE_INSN(rorw, MATCH_RORW, MASK_RORW)
DECLARE_INSN(sbclrw, MATCH_SBCLRW, MASK_SBCLRW)
DECLARE_INSN(sbsetw, MATCH_SBSETW, MASK_SBSETW)
DECLARE_INSN(sbinvw, MATCH_SBINVW, MASK_SBINVW)
DECLARE_INSN(sbextw, MATCH_SBEXTW, MASK_SBEXTW)
DECLARE_INSN(gorcw, MATCH_GORCW, MASK_GORCW)
DECLARE_INSN(grevw, MATCH_GREVW, MASK_GREVW)
DECLARE_INSN(sloiw, MATCH_SLOIW, MASK_SLOIW)
DECLARE_INSN(sroiw, MATCH_SROIW, MASK_SROIW)
DECLARE_INSN(roriw, MATCH_RORIW, MASK_RORIW)
DECLARE_INSN(sbclriw, MATCH_SBCLRIW, MASK_SBCLRIW)
DECLARE_INSN(sbsetiw, MATCH_SBSETIW, MASK_SBSETIW)
DECLARE_INSN(sbinviw, MATCH_SBINVIW, MASK_SBINVIW)
DECLARE_INSN(gorciw, MATCH_GORCIW, MASK_GORCIW)
DECLARE_INSN(greviw, MATCH_GREVIW, MASK_GREVIW)
DECLARE_INSN(fslw, MATCH_FSLW, MASK_FSLW)
DECLARE_INSN(fsrw, MATCH_FSRW, MASK_FSRW)
DECLARE_INSN(fsriw, MATCH_FSRIW, MASK_FSRIW)
DECLARE_INSN(clzw, MATCH_CLZW, MASK_CLZW)
DECLARE_INSN(ctzw, MATCH_CTZW, MASK_CTZW)
DECLARE_INSN(pcntw, MATCH_PCNTW, MASK_PCNTW)
DECLARE_INSN(sh1addu_w, MATCH_SH1ADDU_W, MASK_SH1ADDU_W)
DECLARE_INSN(sh2addu_w, MATCH_SH2ADDU_W, MASK_SH2ADDU_W)
DECLARE_INSN(sh3addu_w, MATCH_SH3ADDU_W, MASK_SH3ADDU_W)
DECLARE_INSN(clmulw, MATCH_CLMULW, MASK_CLMULW)
DECLARE_INSN(clmulrw, MATCH_CLMULRW, MASK_CLMULRW)
DECLARE_INSN(clmulhw, MATCH_CLMULHW, MASK_CLMULHW)
DECLARE_INSN(shflw, MATCH_SHFLW, MASK_SHFLW)
DECLARE_INSN(unshflw, MATCH_UNSHFLW, MASK_UNSHFLW)
DECLARE_INSN(bextw, MATCH_BEXTW, MASK_BEXTW)
DECLARE_INSN(bdepw, MATCH_BDEPW, MASK_BDEPW)
DECLARE_INSN(packw, MATCH_PACKW, MASK_PACKW)
DECLARE_INSN(packuw, MATCH_PACKUW, MASK_PACKUW)
DECLARE_INSN(bfpw, MATCH_BFPW, MASK_BFPW)
DECLARE_INSN(fadd_h, MATCH_FADD_H, MASK_FADD_H)
DECLARE_INSN(fsub_h, MATCH_FSUB_H, MASK_FSUB_H)
DECLARE_INSN(fmul_h, MATCH_FMUL_H, MASK_FMUL_H)
DECLARE_INSN(fdiv_h, MATCH_FDIV_H, MASK_FDIV_H)
DECLARE_INSN(fsgnj_h, MATCH_FSGNJ_H, MASK_FSGNJ_H)
DECLARE_INSN(fsgnjn_h, MATCH_FSGNJN_H, MASK_FSGNJN_H)
DECLARE_INSN(fsgnjx_h, MATCH_FSGNJX_H, MASK_FSGNJX_H)
DECLARE_INSN(fmin_h, MATCH_FMIN_H, MASK_FMIN_H)
DECLARE_INSN(fmax_h, MATCH_FMAX_H, MASK_FMAX_H)
DECLARE_INSN(fcvt_h_s, MATCH_FCVT_H_S, MASK_FCVT_H_S)
DECLARE_INSN(fcvt_s_h, MATCH_FCVT_S_H, MASK_FCVT_S_H)
DECLARE_INSN(fsqrt_h, MATCH_FSQRT_H, MASK_FSQRT_H)
DECLARE_INSN(fle_h, MATCH_FLE_H, MASK_FLE_H)
DECLARE_INSN(flt_h, MATCH_FLT_H, MASK_FLT_H)
DECLARE_INSN(feq_h, MATCH_FEQ_H, MASK_FEQ_H)
DECLARE_INSN(fcvt_w_h, MATCH_FCVT_W_H, MASK_FCVT_W_H)
DECLARE_INSN(fcvt_wu_h, MATCH_FCVT_WU_H, MASK_FCVT_WU_H)
DECLARE_INSN(fmv_x_h, MATCH_FMV_X_H, MASK_FMV_X_H)
DECLARE_INSN(fclass_h, MATCH_FCLASS_H, MASK_FCLASS_H)
DECLARE_INSN(fcvt_h_w, MATCH_FCVT_H_W, MASK_FCVT_H_W)
DECLARE_INSN(fcvt_h_wu, MATCH_FCVT_H_WU, MASK_FCVT_H_WU)
DECLARE_INSN(fmv_h_x, MATCH_FMV_H_X, MASK_FMV_H_X)
DECLARE_INSN(flh, MATCH_FLH, MASK_FLH)
DECLARE_INSN(fsh, MATCH_FSH, MASK_FSH)
DECLARE_INSN(fmadd_h, MATCH_FMADD_H, MASK_FMADD_H)
DECLARE_INSN(fmsub_h, MATCH_FMSUB_H, MASK_FMSUB_H)
DECLARE_INSN(fnmsub_h, MATCH_FNMSUB_H, MASK_FNMSUB_H)
DECLARE_INSN(fnmadd_h, MATCH_FNMADD_H, MASK_FNMADD_H)
DECLARE_INSN(fcvt_h_q, MATCH_FCVT_H_Q, MASK_FCVT_H_Q)
DECLARE_INSN(fcvt_q_h, MATCH_FCVT_Q_H, MASK_FCVT_Q_H)
DECLARE_INSN(fcvt_l_h, MATCH_FCVT_L_H, MASK_FCVT_L_H)
DECLARE_INSN(fcvt_lu_h, MATCH_FCVT_LU_H, MASK_FCVT_LU_H)
DECLARE_INSN(fcvt_h_l, MATCH_FCVT_H_L, MASK_FCVT_H_L)
DECLARE_INSN(fcvt_h_lu, MATCH_FCVT_H_LU, MASK_FCVT_H_LU)
DECLARE_INSN(fcvt_h_d, MATCH_FCVT_H_D, MASK_FCVT_H_D)
DECLARE_INSN(fcvt_d_h, MATCH_FCVT_D_H, MASK_FCVT_D_H)
DECLARE_INSN(fmv_x_q, MATCH_FMV_X_Q, MASK_FMV_X_Q)
DECLARE_INSN(fmv_q_x, MATCH_FMV_Q_X, MASK_FMV_Q_X)
DECLARE_INSN(ecall, MATCH_ECALL, MASK_ECALL)
DECLARE_INSN(ebreak, MATCH_EBREAK, MASK_EBREAK)
DECLARE_INSN(uret, MATCH_URET, MASK_URET)
DECLARE_INSN(sret, MATCH_SRET, MASK_SRET)
DECLARE_INSN(mret, MATCH_MRET, MASK_MRET)
DECLARE_INSN(dret, MATCH_DRET, MASK_DRET)
DECLARE_INSN(sfence_vma, MATCH_SFENCE_VMA, MASK_SFENCE_VMA)
DECLARE_INSN(wfi, MATCH_WFI, MASK_WFI)
DECLARE_INSN(csrrw, MATCH_CSRRW, MASK_CSRRW)
DECLARE_INSN(csrrs, MATCH_CSRRS, MASK_CSRRS)
DECLARE_INSN(csrrc, MATCH_CSRRC, MASK_CSRRC)
DECLARE_INSN(csrrwi, MATCH_CSRRWI, MASK_CSRRWI)
DECLARE_INSN(csrrsi, MATCH_CSRRSI, MASK_CSRRSI)
DECLARE_INSN(csrrci, MATCH_CSRRCI, MASK_CSRRCI)
DECLARE_INSN(c_nop, MATCH_C_NOP, MASK_C_NOP)
DECLARE_INSN(c_addi16sp, MATCH_C_ADDI16SP, MASK_C_ADDI16SP)
DECLARE_INSN(c_jr, MATCH_C_JR, MASK_C_JR)
DECLARE_INSN(c_jalr, MATCH_C_JALR, MASK_C_JALR)
DECLARE_INSN(c_ebreak, MATCH_C_EBREAK, MASK_C_EBREAK)
DECLARE_INSN(c_addi4spn, MATCH_C_ADDI4SPN, MASK_C_ADDI4SPN)
DECLARE_INSN(c_fld, MATCH_C_FLD, MASK_C_FLD)
DECLARE_INSN(c_lw, MATCH_C_LW, MASK_C_LW)
DECLARE_INSN(c_flw, MATCH_C_FLW, MASK_C_FLW)
DECLARE_INSN(c_fsd, MATCH_C_FSD, MASK_C_FSD)
DECLARE_INSN(c_sw, MATCH_C_SW, MASK_C_SW)
DECLARE_INSN(c_fsw, MATCH_C_FSW, MASK_C_FSW)
DECLARE_INSN(c_addi, MATCH_C_ADDI, MASK_C_ADDI)
DECLARE_INSN(c_jal, MATCH_C_JAL, MASK_C_JAL)
DECLARE_INSN(c_li, MATCH_C_LI, MASK_C_LI)
DECLARE_INSN(c_lui, MATCH_C_LUI, MASK_C_LUI)
DECLARE_INSN(c_srli, MATCH_C_SRLI, MASK_C_SRLI)
DECLARE_INSN(c_srai, MATCH_C_SRAI, MASK_C_SRAI)
DECLARE_INSN(c_andi, MATCH_C_ANDI, MASK_C_ANDI)
DECLARE_INSN(c_sub, MATCH_C_SUB, MASK_C_SUB)
DECLARE_INSN(c_xor, MATCH_C_XOR, MASK_C_XOR)
DECLARE_INSN(c_or, MATCH_C_OR, MASK_C_OR)
DECLARE_INSN(c_and, MATCH_C_AND, MASK_C_AND)
DECLARE_INSN(c_j, MATCH_C_J, MASK_C_J)
DECLARE_INSN(c_beqz, MATCH_C_BEQZ, MASK_C_BEQZ)
DECLARE_INSN(c_bnez, MATCH_C_BNEZ, MASK_C_BNEZ)
DECLARE_INSN(c_slli, MATCH_C_SLLI, MASK_C_SLLI)
DECLARE_INSN(c_fldsp, MATCH_C_FLDSP, MASK_C_FLDSP)
DECLARE_INSN(c_lwsp, MATCH_C_LWSP, MASK_C_LWSP)
DECLARE_INSN(c_flwsp, MATCH_C_FLWSP, MASK_C_FLWSP)
DECLARE_INSN(c_mv, MATCH_C_MV, MASK_C_MV)
DECLARE_INSN(c_add, MATCH_C_ADD, MASK_C_ADD)
DECLARE_INSN(c_fsdsp, MATCH_C_FSDSP, MASK_C_FSDSP)
DECLARE_INSN(c_swsp, MATCH_C_SWSP, MASK_C_SWSP)
DECLARE_INSN(c_fswsp, MATCH_C_FSWSP, MASK_C_FSWSP)
DECLARE_INSN(c_srli_rv32, MATCH_C_SRLI_RV32, MASK_C_SRLI_RV32)
DECLARE_INSN(c_srai_rv32, MATCH_C_SRAI_RV32, MASK_C_SRAI_RV32)
DECLARE_INSN(c_slli_rv32, MATCH_C_SLLI_RV32, MASK_C_SLLI_RV32)
DECLARE_INSN(c_ld, MATCH_C_LD, MASK_C_LD)
DECLARE_INSN(c_sd, MATCH_C_SD, MASK_C_SD)
DECLARE_INSN(c_subw, MATCH_C_SUBW, MASK_C_SUBW)
DECLARE_INSN(c_addw, MATCH_C_ADDW, MASK_C_ADDW)
DECLARE_INSN(c_addiw, MATCH_C_ADDIW, MASK_C_ADDIW)
DECLARE_INSN(c_ldsp, MATCH_C_LDSP, MASK_C_LDSP)
DECLARE_INSN(c_sdsp, MATCH_C_SDSP, MASK_C_SDSP)
DECLARE_INSN(custom0, MATCH_CUSTOM0, MASK_CUSTOM0)
DECLARE_INSN(custom0_rs1, MATCH_CUSTOM0_RS1, MASK_CUSTOM0_RS1)
DECLARE_INSN(custom0_rs1_rs2, MATCH_CUSTOM0_RS1_RS2, MASK_CUSTOM0_RS1_RS2)
DECLARE_INSN(custom0_rd, MATCH_CUSTOM0_RD, MASK_CUSTOM0_RD)
DECLARE_INSN(custom0_rd_rs1, MATCH_CUSTOM0_RD_RS1, MASK_CUSTOM0_RD_RS1)
DECLARE_INSN(custom0_rd_rs1_rs2, MATCH_CUSTOM0_RD_RS1_RS2, MASK_CUSTOM0_RD_RS1_RS2)
DECLARE_INSN(custom1, MATCH_CUSTOM1, MASK_CUSTOM1)
DECLARE_INSN(custom1_rs1, MATCH_CUSTOM1_RS1, MASK_CUSTOM1_RS1)
DECLARE_INSN(custom1_rs1_rs2, MATCH_CUSTOM1_RS1_RS2, MASK_CUSTOM1_RS1_RS2)
DECLARE_INSN(custom1_rd, MATCH_CUSTOM1_RD, MASK_CUSTOM1_RD)
DECLARE_INSN(custom1_rd_rs1, MATCH_CUSTOM1_RD_RS1, MASK_CUSTOM1_RD_RS1)
DECLARE_INSN(custom1_rd_rs1_rs2, MATCH_CUSTOM1_RD_RS1_RS2, MASK_CUSTOM1_RD_RS1_RS2)
DECLARE_INSN(custom2, MATCH_CUSTOM2, MASK_CUSTOM2)
DECLARE_INSN(custom2_rs1, MATCH_CUSTOM2_RS1, MASK_CUSTOM2_RS1)
DECLARE_INSN(custom2_rs1_rs2, MATCH_CUSTOM2_RS1_RS2, MASK_CUSTOM2_RS1_RS2)
DECLARE_INSN(custom2_rd, MATCH_CUSTOM2_RD, MASK_CUSTOM2_RD)
DECLARE_INSN(custom2_rd_rs1, MATCH_CUSTOM2_RD_RS1, MASK_CUSTOM2_RD_RS1)
DECLARE_INSN(custom2_rd_rs1_rs2, MATCH_CUSTOM2_RD_RS1_RS2, MASK_CUSTOM2_RD_RS1_RS2)
DECLARE_INSN(custom3, MATCH_CUSTOM3, MASK_CUSTOM3)
DECLARE_INSN(custom3_rs1, MATCH_CUSTOM3_RS1, MASK_CUSTOM3_RS1)
DECLARE_INSN(custom3_rs1_rs2, MATCH_CUSTOM3_RS1_RS2, MASK_CUSTOM3_RS1_RS2)
DECLARE_INSN(custom3_rd, MATCH_CUSTOM3_RD, MASK_CUSTOM3_RD)
DECLARE_INSN(custom3_rd_rs1, MATCH_CUSTOM3_RD_RS1, MASK_CUSTOM3_RD_RS1)
DECLARE_INSN(custom3_rd_rs1_rs2, MATCH_CUSTOM3_RD_RS1_RS2, MASK_CUSTOM3_RD_RS1_RS2)
DECLARE_INSN(vecvt_hf_x8_m, MATCH_VECVT_HF_X8_M, MASK_VECVT_HF_X8_M)
DECLARE_INSN(vecvt_hf_xu8_m, MATCH_VECVT_HF_XU8_M, MASK_VECVT_HF_XU8_M)
DECLARE_INSN(vecvt_x8_hf_m, MATCH_VECVT_X8_HF_M, MASK_VECVT_X8_HF_M)
DECLARE_INSN(vecvt_xu8_hf_m, MATCH_VECVT_XU8_HF_M, MASK_VECVT_XU8_HF_M)
DECLARE_INSN(vecvt_hf_x16_m, MATCH_VECVT_HF_X16_M, MASK_VECVT_HF_X16_M)
DECLARE_INSN(vecvt_x16_hf_m, MATCH_VECVT_X16_HF_M, MASK_VECVT_X16_HF_M)
DECLARE_INSN(vecvt_f32_hf_m, MATCH_VECVT_F32_HF_M, MASK_VECVT_F32_HF_M)
DECLARE_INSN(vecvt_hf_f32_m, MATCH_VECVT_HF_F32_M, MASK_VECVT_HF_F32_M)
DECLARE_INSN(vecvt_bf_x8_m, MATCH_VECVT_BF_X8_M, MASK_VECVT_BF_X8_M)
DECLARE_INSN(vecvt_bf_xu8_m, MATCH_VECVT_BF_XU8_M, MASK_VECVT_BF_XU8_M)
DECLARE_INSN(vecvt_x8_bf_m, MATCH_VECVT_X8_BF_M, MASK_VECVT_X8_BF_M)
DECLARE_INSN(vecvt_xu8_bf_m, MATCH_VECVT_XU8_BF_M, MASK_VECVT_XU8_BF_M)
DECLARE_INSN(vecvt_bf_x16_m, MATCH_VECVT_BF_X16_M, MASK_VECVT_BF_X16_M)
DECLARE_INSN(vecvt_x16_bf_m, MATCH_VECVT_X16_BF_M, MASK_VECVT_X16_BF_M)
DECLARE_INSN(vecvt_f32_bf_m, MATCH_VECVT_F32_BF_M, MASK_VECVT_F32_BF_M)
DECLARE_INSN(vecvt_bf_f32_m, MATCH_VECVT_BF_F32_M, MASK_VECVT_BF_F32_M)
DECLARE_INSN(vecvt_bf_hf_m, MATCH_VECVT_BF_HF_M, MASK_VECVT_BF_HF_M)
DECLARE_INSN(vecvt_hf_bf_m, MATCH_VECVT_HF_BF_M, MASK_VECVT_HF_BF_M)
DECLARE_INSN(vecvt_f32_x32_m, MATCH_VECVT_F32_X32_M, MASK_VECVT_F32_X32_M)
DECLARE_INSN(vecvt_x32_f32_m, MATCH_VECVT_X32_F32_M, MASK_VECVT_X32_F32_M)
DECLARE_INSN(veadd_mm, MATCH_VEADD_MM, MASK_VEADD_MM)
DECLARE_INSN(veadd_mv, MATCH_VEADD_MV, MASK_VEADD_MV)
DECLARE_INSN(veadd_mf, MATCH_VEADD_MF, MASK_VEADD_MF)
DECLARE_INSN(veadd_relu_mm, MATCH_VEADD_RELU_MM, MASK_VEADD_RELU_MM)
DECLARE_INSN(veadd_relu_mv, MATCH_VEADD_RELU_MV, MASK_VEADD_RELU_MV)
DECLARE_INSN(veadd_relu_mf, MATCH_VEADD_RELU_MF, MASK_VEADD_RELU_MF)
DECLARE_INSN(vesub_mm, MATCH_VESUB_MM, MASK_VESUB_MM)
DECLARE_INSN(vesub_mv, MATCH_VESUB_MV, MASK_VESUB_MV)
DECLARE_INSN(vesub_mf, MATCH_VESUB_MF, MASK_VESUB_MF)
DECLARE_INSN(veemul_mm, MATCH_VEEMUL_MM, MASK_VEEMUL_MM)
DECLARE_INSN(veemul_mv, MATCH_VEEMUL_MV, MASK_VEEMUL_MV)
DECLARE_INSN(veemul_mf, MATCH_VEEMUL_MF, MASK_VEEMUL_MF)
DECLARE_INSN(veemul_relu_mv, MATCH_VEEMUL_RELU_MV, MASK_VEEMUL_RELU_MV)
DECLARE_INSN(veemul_x8_hf_mf, MATCH_VEEMUL_X8_HF_MF, MASK_VEEMUL_X8_HF_MF)
DECLARE_INSN(veemul_xu8_hf_mf, MATCH_VEEMUL_XU8_HF_MF, MASK_VEEMUL_XU8_HF_MF)
DECLARE_INSN(veemul_x8_bf_mf, MATCH_VEEMUL_X8_BF_MF, MASK_VEEMUL_X8_BF_MF)
DECLARE_INSN(veemul_xu8_bf_mf, MATCH_VEEMUL_XU8_BF_MF, MASK_VEEMUL_XU8_BF_MF)
DECLARE_INSN(veemul_relu_mm, MATCH_VEEMUL_RELU_MM, MASK_VEEMUL_RELU_MM)
DECLARE_INSN(veemul_relu_mf, MATCH_VEEMUL_RELU_MF, MASK_VEEMUL_RELU_MF)
DECLARE_INSN(veemacc_mm, MATCH_VEEMACC_MM, MASK_VEEMACC_MM)
DECLARE_INSN(veemacc_mv, MATCH_VEEMACC_MV, MASK_VEEMACC_MV)
DECLARE_INSN(veemacc_mf, MATCH_VEEMACC_MF, MASK_VEEMACC_MF)
DECLARE_INSN(veacc_m, MATCH_VEACC_M, MASK_VEACC_M)
DECLARE_INSN(vemax_m, MATCH_VEMAX_M, MASK_VEMAX_M)
DECLARE_INSN(vemin_m, MATCH_VEMIN_M, MASK_VEMIN_M)
DECLARE_INSN(veargmax_m, MATCH_VEARGMAX_M, MASK_VEARGMAX_M)
DECLARE_INSN(veargmin_m, MATCH_VEARGMIN_M, MASK_VEARGMIN_M)
DECLARE_INSN(vemax_mm, MATCH_VEMAX_MM, MASK_VEMAX_MM)
DECLARE_INSN(vemax_mv, MATCH_VEMAX_MV, MASK_VEMAX_MV)
DECLARE_INSN(vemax_mf, MATCH_VEMAX_MF, MASK_VEMAX_MF)
DECLARE_INSN(vemin_mm, MATCH_VEMIN_MM, MASK_VEMIN_MM)
DECLARE_INSN(vemin_mv, MATCH_VEMIN_MV, MASK_VEMIN_MV)
DECLARE_INSN(vemin_mf, MATCH_VEMIN_MF, MASK_VEMIN_MF)
DECLARE_INSN(velkrelu_mv, MATCH_VELKRELU_MV, MASK_VELKRELU_MV)
DECLARE_INSN(velkrelu_mf, MATCH_VELKRELU_MF, MASK_VELKRELU_MF)
DECLARE_INSN(velut_m, MATCH_VELUT_M, MASK_VELUT_M)
DECLARE_INSN(mov_m, MATCH_MOV_M, MASK_MOV_M)
DECLARE_INSN(mov_llb_l1, MATCH_MOV_LLB_L1, MASK_MOV_LLB_L1)
DECLARE_INSN(mov_l1_llb, MATCH_MOV_L1_LLB, MASK_MOV_L1_LLB)
DECLARE_INSN(mov_l1_glb, MATCH_MOV_L1_GLB, MASK_MOV_L1_GLB)
DECLARE_INSN(mov_llb_glb, MATCH_MOV_LLB_GLB, MASK_MOV_LLB_GLB)
DECLARE_INSN(mov_glb_llb, MATCH_MOV_GLB_LLB, MASK_MOV_GLB_LLB)
DECLARE_INSN(mov_glb_l1, MATCH_MOV_GLB_L1, MASK_MOV_GLB_L1)
DECLARE_INSN(mov_glb_glb, MATCH_MOV_GLB_GLB, MASK_MOV_GLB_GLB)
DECLARE_INSN(mov_llb_llb, MATCH_MOV_LLB_LLB, MASK_MOV_LLB_LLB)
DECLARE_INSN(mov_v, MATCH_MOV_V, MASK_MOV_V)
DECLARE_INSN(mov_f, MATCH_MOV_F, MASK_MOV_F)
DECLARE_INSN(icmov_m, MATCH_ICMOV_M, MASK_ICMOV_M)
DECLARE_INSN(sync, MATCH_SYNC, MASK_SYNC)
DECLARE_INSN(sync_mte, MATCH_SYNC_MTE, MASK_SYNC_MTE)
DECLARE_INSN(sync_dmae, MATCH_SYNC_DMAE, MASK_SYNC_DMAE)
DECLARE_INSN(sync_mte_r, MATCH_SYNC_MTE_R, MASK_SYNC_MTE_R)
DECLARE_INSN(sync_mte_w, MATCH_SYNC_MTE_W, MASK_SYNC_MTE_W)
DECLARE_INSN(sync_dmae_r, MATCH_SYNC_DMAE_R, MASK_SYNC_DMAE_R)
DECLARE_INSN(sync_dmae_w, MATCH_SYNC_DMAE_W, MASK_SYNC_DMAE_W)
DECLARE_INSN(flhw, MATCH_FLHW, MASK_FLHW)
DECLARE_INSN(fshw, MATCH_FSHW, MASK_FSHW)
DECLARE_INSN(metr_m, MATCH_METR_M, MASK_METR_M)
DECLARE_INSN(meconv_mm, MATCH_MECONV_MM, MASK_MECONV_MM)
DECLARE_INSN(meconv_sp_mm, MATCH_MECONV_SP_MM, MASK_MECONV_SP_MM)
DECLARE_INSN(medeconv_mm, MATCH_MEDECONV_MM, MASK_MEDECONV_MM)
DECLARE_INSN(medeconv_sp_mm, MATCH_MEDECONV_SP_MM, MASK_MEDECONV_SP_MM)
DECLARE_INSN(pld, MATCH_PLD, MASK_PLD)
DECLARE_INSN(veexp_m, MATCH_VEEXP_M, MASK_VEEXP_M)
DECLARE_INSN(vesqrt_m, MATCH_VESQRT_M, MASK_VESQRT_M)
DECLARE_INSN(verecip_m, MATCH_VERECIP_M, MASK_VERECIP_M)
DECLARE_INSN(versqrt_m, MATCH_VERSQRT_M, MASK_VERSQRT_M)
DECLARE_INSN(veln_m, MATCH_VELN_M, MASK_VELN_M)
DECLARE_INSN(vesin_m, MATCH_VESIN_M, MASK_VESIN_M)
DECLARE_INSN(vecos_m, MATCH_VECOS_M, MASK_VECOS_M)
DECLARE_INSN(vetanh_m, MATCH_VETANH_M, MASK_VETANH_M)
DECLARE_INSN(vesigmoid_m, MATCH_VESIGMOID_M, MASK_VESIGMOID_M)
DECLARE_INSN(vesinh_m, MATCH_VESINH_M, MASK_VESINH_M)
DECLARE_INSN(vecosh_m, MATCH_VECOSH_M, MASK_VECOSH_M)
DECLARE_INSN(vediv_mm, MATCH_VEDIV_MM, MASK_VEDIV_MM)
DECLARE_INSN(vediv_mv, MATCH_VEDIV_MV, MASK_VEDIV_MV)
DECLARE_INSN(vediv_mf, MATCH_VEDIV_MF, MASK_VEDIV_MF)
DECLARE_INSN(veavgpool_m, MATCH_VEAVGPOOL_M, MASK_VEAVGPOOL_M)
DECLARE_INSN(vemaxpool_m, MATCH_VEMAXPOOL_M, MASK_VEMAXPOOL_M)
DECLARE_INSN(vedwconv_mm, MATCH_VEDWCONV_MM, MASK_VEDWCONV_MM)
DECLARE_INSN(versub_mv, MATCH_VERSUB_MV, MASK_VERSUB_MV)
DECLARE_INSN(versub_mf, MATCH_VERSUB_MF, MASK_VERSUB_MF)
DECLARE_INSN(verot180_m, MATCH_VEROT180_M, MASK_VEROT180_M)
DECLARE_INSN(verev_m, MATCH_VEREV_M, MASK_VEREV_M)
DECLARE_INSN(verand_v, MATCH_VERAND_V, MASK_VERAND_V)
DECLARE_INSN(verand_m, MATCH_VERAND_M, MASK_VERAND_M)
DECLARE_INSN(memul_mm, MATCH_MEMUL_MM, MASK_MEMUL_MM)
DECLARE_INSN(memul_sp_mm, MATCH_MEMUL_SP_MM, MASK_MEMUL_SP_MM)
DECLARE_INSN(memul_ts1_mm, MATCH_MEMUL_TS1_MM, MASK_MEMUL_TS1_MM)
DECLARE_INSN(memin_m, MATCH_MEMIN_M, MASK_MEMIN_M)
DECLARE_INSN(memax_m, MATCH_MEMAX_M, MASK_MEMAX_M)
DECLARE_INSN(meacc_m, MATCH_MEACC_M, MASK_MEACC_M)
DECLARE_INSN(medwconv_mm, MATCH_MEDWCONV_MM, MASK_MEDWCONV_MM)
DECLARE_INSN(vsetvli, MATCH_VSETVLI, MASK_VSETVLI)
DECLARE_INSN(vsetvl, MATCH_VSETVL, MASK_VSETVL)
DECLARE_INSN(vle8_v, MATCH_VLE8_V, MASK_VLE8_V)
DECLARE_INSN(vle8pi_v, MATCH_VLE8PI_V, MASK_VLE8PI_V)
DECLARE_INSN(vle16_v, MATCH_VLE16_V, MASK_VLE16_V)
DECLARE_INSN(vle16pi_v, MATCH_VLE16PI_V, MASK_VLE16PI_V)
DECLARE_INSN(vle32_v, MATCH_VLE32_V, MASK_VLE32_V)
DECLARE_INSN(vle32pi_v, MATCH_VLE32PI_V, MASK_VLE32PI_V)
DECLARE_INSN(vle64_v, MATCH_VLE64_V, MASK_VLE64_V)
DECLARE_INSN(vle128_v, MATCH_VLE128_V, MASK_VLE128_V)
DECLARE_INSN(vle256_v, MATCH_VLE256_V, MASK_VLE256_V)
DECLARE_INSN(vle512_v, MATCH_VLE512_V, MASK_VLE512_V)
DECLARE_INSN(vle1024_v, MATCH_VLE1024_V, MASK_VLE1024_V)
DECLARE_INSN(vse8_v, MATCH_VSE8_V, MASK_VSE8_V)
DECLARE_INSN(vse8pi_v, MATCH_VSE8PI_V, MASK_VSE8PI_V)
DECLARE_INSN(vse16_v, MATCH_VSE16_V, MASK_VSE16_V)
DECLARE_INSN(vse16pi_v, MATCH_VSE16PI_V, MASK_VSE16PI_V)
DECLARE_INSN(vse32_v, MATCH_VSE32_V, MASK_VSE32_V)
DECLARE_INSN(vse32pi_v, MATCH_VSE32PI_V, MASK_VSE32PI_V)
DECLARE_INSN(vse64_v, MATCH_VSE64_V, MASK_VSE64_V)
DECLARE_INSN(vse128_v, MATCH_VSE128_V, MASK_VSE128_V)
DECLARE_INSN(vse256_v, MATCH_VSE256_V, MASK_VSE256_V)
DECLARE_INSN(vse512_v, MATCH_VSE512_V, MASK_VSE512_V)
DECLARE_INSN(vse1024_v, MATCH_VSE1024_V, MASK_VSE1024_V)
DECLARE_INSN(vluxei8_v, MATCH_VLUXEI8_V, MASK_VLUXEI8_V)
DECLARE_INSN(vluxei16_v, MATCH_VLUXEI16_V, MASK_VLUXEI16_V)
DECLARE_INSN(vluxei32_v, MATCH_VLUXEI32_V, MASK_VLUXEI32_V)
DECLARE_INSN(vluxei64_v, MATCH_VLUXEI64_V, MASK_VLUXEI64_V)
DECLARE_INSN(vluxei128_v, MATCH_VLUXEI128_V, MASK_VLUXEI128_V)
DECLARE_INSN(vluxei256_v, MATCH_VLUXEI256_V, MASK_VLUXEI256_V)
DECLARE_INSN(vluxei512_v, MATCH_VLUXEI512_V, MASK_VLUXEI512_V)
DECLARE_INSN(vluxei1024_v, MATCH_VLUXEI1024_V, MASK_VLUXEI1024_V)
DECLARE_INSN(vsuxei8_v, MATCH_VSUXEI8_V, MASK_VSUXEI8_V)
DECLARE_INSN(vsuxei16_v, MATCH_VSUXEI16_V, MASK_VSUXEI16_V)
DECLARE_INSN(vsuxei32_v, MATCH_VSUXEI32_V, MASK_VSUXEI32_V)
DECLARE_INSN(vsuxei64_v, MATCH_VSUXEI64_V, MASK_VSUXEI64_V)
DECLARE_INSN(vsuxei128_v, MATCH_VSUXEI128_V, MASK_VSUXEI128_V)
DECLARE_INSN(vsuxei256_v, MATCH_VSUXEI256_V, MASK_VSUXEI256_V)
DECLARE_INSN(vsuxei512_v, MATCH_VSUXEI512_V, MASK_VSUXEI512_V)
DECLARE_INSN(vsuxei1024_v, MATCH_VSUXEI1024_V, MASK_VSUXEI1024_V)
DECLARE_INSN(vlse8_v, MATCH_VLSE8_V, MASK_VLSE8_V)
DECLARE_INSN(vlse16_v, MATCH_VLSE16_V, MASK_VLSE16_V)
DECLARE_INSN(vlse32_v, MATCH_VLSE32_V, MASK_VLSE32_V)
DECLARE_INSN(vlse64_v, MATCH_VLSE64_V, MASK_VLSE64_V)
DECLARE_INSN(vlse128_v, MATCH_VLSE128_V, MASK_VLSE128_V)
DECLARE_INSN(vlse256_v, MATCH_VLSE256_V, MASK_VLSE256_V)
DECLARE_INSN(vlse512_v, MATCH_VLSE512_V, MASK_VLSE512_V)
DECLARE_INSN(vlse1024_v, MATCH_VLSE1024_V, MASK_VLSE1024_V)
DECLARE_INSN(vsse8_v, MATCH_VSSE8_V, MASK_VSSE8_V)
DECLARE_INSN(vsse16_v, MATCH_VSSE16_V, MASK_VSSE16_V)
DECLARE_INSN(vsse32_v, MATCH_VSSE32_V, MASK_VSSE32_V)
DECLARE_INSN(vsse64_v, MATCH_VSSE64_V, MASK_VSSE64_V)
DECLARE_INSN(vsse128_v, MATCH_VSSE128_V, MASK_VSSE128_V)
DECLARE_INSN(vsse256_v, MATCH_VSSE256_V, MASK_VSSE256_V)
DECLARE_INSN(vsse512_v, MATCH_VSSE512_V, MASK_VSSE512_V)
DECLARE_INSN(vsse1024_v, MATCH_VSSE1024_V, MASK_VSSE1024_V)
DECLARE_INSN(vloxei8_v, MATCH_VLOXEI8_V, MASK_VLOXEI8_V)
DECLARE_INSN(vloxei16_v, MATCH_VLOXEI16_V, MASK_VLOXEI16_V)
DECLARE_INSN(vloxei32_v, MATCH_VLOXEI32_V, MASK_VLOXEI32_V)
DECLARE_INSN(vloxei64_v, MATCH_VLOXEI64_V, MASK_VLOXEI64_V)
DECLARE_INSN(vloxei128_v, MATCH_VLOXEI128_V, MASK_VLOXEI128_V)
DECLARE_INSN(vloxei256_v, MATCH_VLOXEI256_V, MASK_VLOXEI256_V)
DECLARE_INSN(vloxei512_v, MATCH_VLOXEI512_V, MASK_VLOXEI512_V)
DECLARE_INSN(vloxei1024_v, MATCH_VLOXEI1024_V, MASK_VLOXEI1024_V)
DECLARE_INSN(vsoxei8_v, MATCH_VSOXEI8_V, MASK_VSOXEI8_V)
DECLARE_INSN(vsoxei16_v, MATCH_VSOXEI16_V, MASK_VSOXEI16_V)
DECLARE_INSN(vsoxei32_v, MATCH_VSOXEI32_V, MASK_VSOXEI32_V)
DECLARE_INSN(vsoxei64_v, MATCH_VSOXEI64_V, MASK_VSOXEI64_V)
DECLARE_INSN(vsoxei128_v, MATCH_VSOXEI128_V, MASK_VSOXEI128_V)
DECLARE_INSN(vsoxei256_v, MATCH_VSOXEI256_V, MASK_VSOXEI256_V)
DECLARE_INSN(vsoxei512_v, MATCH_VSOXEI512_V, MASK_VSOXEI512_V)
DECLARE_INSN(vsoxei1024_v, MATCH_VSOXEI1024_V, MASK_VSOXEI1024_V)
DECLARE_INSN(vle8ff_v, MATCH_VLE8FF_V, MASK_VLE8FF_V)
DECLARE_INSN(vle16ff_v, MATCH_VLE16FF_V, MASK_VLE16FF_V)
DECLARE_INSN(vle32ff_v, MATCH_VLE32FF_V, MASK_VLE32FF_V)
DECLARE_INSN(vle64ff_v, MATCH_VLE64FF_V, MASK_VLE64FF_V)
DECLARE_INSN(vle128ff_v, MATCH_VLE128FF_V, MASK_VLE128FF_V)
DECLARE_INSN(vle256ff_v, MATCH_VLE256FF_V, MASK_VLE256FF_V)
DECLARE_INSN(vle512ff_v, MATCH_VLE512FF_V, MASK_VLE512FF_V)
DECLARE_INSN(vle1024ff_v, MATCH_VLE1024FF_V, MASK_VLE1024FF_V)
DECLARE_INSN(vl1re8_v, MATCH_VL1RE8_V, MASK_VL1RE8_V)
DECLARE_INSN(vl1re16_v, MATCH_VL1RE16_V, MASK_VL1RE16_V)
DECLARE_INSN(vl1re32_v, MATCH_VL1RE32_V, MASK_VL1RE32_V)
DECLARE_INSN(vl1re64_v, MATCH_VL1RE64_V, MASK_VL1RE64_V)
DECLARE_INSN(vl2re8_v, MATCH_VL2RE8_V, MASK_VL2RE8_V)
DECLARE_INSN(vl2re16_v, MATCH_VL2RE16_V, MASK_VL2RE16_V)
DECLARE_INSN(vl2re32_v, MATCH_VL2RE32_V, MASK_VL2RE32_V)
DECLARE_INSN(vl2re64_v, MATCH_VL2RE64_V, MASK_VL2RE64_V)
DECLARE_INSN(vl4re8_v, MATCH_VL4RE8_V, MASK_VL4RE8_V)
DECLARE_INSN(vl4re16_v, MATCH_VL4RE16_V, MASK_VL4RE16_V)
DECLARE_INSN(vl4re32_v, MATCH_VL4RE32_V, MASK_VL4RE32_V)
DECLARE_INSN(vl4re64_v, MATCH_VL4RE64_V, MASK_VL4RE64_V)
DECLARE_INSN(vl8re8_v, MATCH_VL8RE8_V, MASK_VL8RE8_V)
DECLARE_INSN(vl8re16_v, MATCH_VL8RE16_V, MASK_VL8RE16_V)
DECLARE_INSN(vl8re32_v, MATCH_VL8RE32_V, MASK_VL8RE32_V)
DECLARE_INSN(vl8re64_v, MATCH_VL8RE64_V, MASK_VL8RE64_V)
DECLARE_INSN(vs1r_v, MATCH_VS1R_V, MASK_VS1R_V)
DECLARE_INSN(vs2r_v, MATCH_VS2R_V, MASK_VS2R_V)
DECLARE_INSN(vs4r_v, MATCH_VS4R_V, MASK_VS4R_V)
DECLARE_INSN(vs8r_v, MATCH_VS8R_V, MASK_VS8R_V)
DECLARE_INSN(vfadd_vf, MATCH_VFADD_VF, MASK_VFADD_VF)
DECLARE_INSN(vfsub_vf, MATCH_VFSUB_VF, MASK_VFSUB_VF)
DECLARE_INSN(vfmin_vf, MATCH_VFMIN_VF, MASK_VFMIN_VF)
DECLARE_INSN(vfmax_vf, MATCH_VFMAX_VF, MASK_VFMAX_VF)
DECLARE_INSN(vfsgnj_vf, MATCH_VFSGNJ_VF, MASK_VFSGNJ_VF)
DECLARE_INSN(vfsgnjn_vf, MATCH_VFSGNJN_VF, MASK_VFSGNJN_VF)
DECLARE_INSN(vfsgnjx_vf, MATCH_VFSGNJX_VF, MASK_VFSGNJX_VF)
DECLARE_INSN(vfslide1up_vf, MATCH_VFSLIDE1UP_VF, MASK_VFSLIDE1UP_VF)
DECLARE_INSN(vfslide1down_vf, MATCH_VFSLIDE1DOWN_VF, MASK_VFSLIDE1DOWN_VF)
DECLARE_INSN(vfmv_s_f, MATCH_VFMV_S_F, MASK_VFMV_S_F)
DECLARE_INSN(vfmerge_vfm, MATCH_VFMERGE_VFM, MASK_VFMERGE_VFM)
DECLARE_INSN(vfmv_v_f, MATCH_VFMV_V_F, MASK_VFMV_V_F)
DECLARE_INSN(vmfeq_vf, MATCH_VMFEQ_VF, MASK_VMFEQ_VF)
DECLARE_INSN(vmfle_vf, MATCH_VMFLE_VF, MASK_VMFLE_VF)
DECLARE_INSN(vmflt_vf, MATCH_VMFLT_VF, MASK_VMFLT_VF)
DECLARE_INSN(vmfne_vf, MATCH_VMFNE_VF, MASK_VMFNE_VF)
DECLARE_INSN(vmfgt_vf, MATCH_VMFGT_VF, MASK_VMFGT_VF)
DECLARE_INSN(vmfge_vf, MATCH_VMFGE_VF, MASK_VMFGE_VF)
DECLARE_INSN(vfdiv_vf, MATCH_VFDIV_VF, MASK_VFDIV_VF)
DECLARE_INSN(vfrdiv_vf, MATCH_VFRDIV_VF, MASK_VFRDIV_VF)
DECLARE_INSN(vfmul_vf, MATCH_VFMUL_VF, MASK_VFMUL_VF)
DECLARE_INSN(vfrsub_vf, MATCH_VFRSUB_VF, MASK_VFRSUB_VF)
DECLARE_INSN(vfmadd_vf, MATCH_VFMADD_VF, MASK_VFMADD_VF)
DECLARE_INSN(vfnmadd_vf, MATCH_VFNMADD_VF, MASK_VFNMADD_VF)
DECLARE_INSN(vfmsub_vf, MATCH_VFMSUB_VF, MASK_VFMSUB_VF)
DECLARE_INSN(vfnmsub_vf, MATCH_VFNMSUB_VF, MASK_VFNMSUB_VF)
DECLARE_INSN(vfmacc_vf, MATCH_VFMACC_VF, MASK_VFMACC_VF)
DECLARE_INSN(vfnmacc_vf, MATCH_VFNMACC_VF, MASK_VFNMACC_VF)
DECLARE_INSN(vfmsac_vf, MATCH_VFMSAC_VF, MASK_VFMSAC_VF)
DECLARE_INSN(vfnmsac_vf, MATCH_VFNMSAC_VF, MASK_VFNMSAC_VF)
DECLARE_INSN(vfwadd_vf, MATCH_VFWADD_VF, MASK_VFWADD_VF)
DECLARE_INSN(vfwsub_vf, MATCH_VFWSUB_VF, MASK_VFWSUB_VF)
DECLARE_INSN(vfwadd_wf, MATCH_VFWADD_WF, MASK_VFWADD_WF)
DECLARE_INSN(vfwsub_wf, MATCH_VFWSUB_WF, MASK_VFWSUB_WF)
DECLARE_INSN(vfwmul_vf, MATCH_VFWMUL_VF, MASK_VFWMUL_VF)
DECLARE_INSN(vfwmacc_vf, MATCH_VFWMACC_VF, MASK_VFWMACC_VF)
DECLARE_INSN(vfwnmacc_vf, MATCH_VFWNMACC_VF, MASK_VFWNMACC_VF)
DECLARE_INSN(vfwmsac_vf, MATCH_VFWMSAC_VF, MASK_VFWMSAC_VF)
DECLARE_INSN(vfwnmsac_vf, MATCH_VFWNMSAC_VF, MASK_VFWNMSAC_VF)
DECLARE_INSN(vfadd_vv, MATCH_VFADD_VV, MASK_VFADD_VV)
DECLARE_INSN(vfredsum_vs, MATCH_VFREDSUM_VS, MASK_VFREDSUM_VS)
DECLARE_INSN(vfsub_vv, MATCH_VFSUB_VV, MASK_VFSUB_VV)
DECLARE_INSN(vfredosum_vs, MATCH_VFREDOSUM_VS, MASK_VFREDOSUM_VS)
DECLARE_INSN(vfmin_vv, MATCH_VFMIN_VV, MASK_VFMIN_VV)
DECLARE_INSN(vfredmin_vs, MATCH_VFREDMIN_VS, MASK_VFREDMIN_VS)
DECLARE_INSN(vfmax_vv, MATCH_VFMAX_VV, MASK_VFMAX_VV)
DECLARE_INSN(vfredmax_vs, MATCH_VFREDMAX_VS, MASK_VFREDMAX_VS)
DECLARE_INSN(vfsgnj_vv, MATCH_VFSGNJ_VV, MASK_VFSGNJ_VV)
DECLARE_INSN(vfsgnjn_vv, MATCH_VFSGNJN_VV, MASK_VFSGNJN_VV)
DECLARE_INSN(vfsgnjx_vv, MATCH_VFSGNJX_VV, MASK_VFSGNJX_VV)
DECLARE_INSN(vfmv_f_s, MATCH_VFMV_F_S, MASK_VFMV_F_S)
DECLARE_INSN(vmfeq_vv, MATCH_VMFEQ_VV, MASK_VMFEQ_VV)
DECLARE_INSN(vmfle_vv, MATCH_VMFLE_VV, MASK_VMFLE_VV)
DECLARE_INSN(vmflt_vv, MATCH_VMFLT_VV, MASK_VMFLT_VV)
DECLARE_INSN(vmfne_vv, MATCH_VMFNE_VV, MASK_VMFNE_VV)
DECLARE_INSN(vfdiv_vv, MATCH_VFDIV_VV, MASK_VFDIV_VV)
DECLARE_INSN(vfmul_vv, MATCH_VFMUL_VV, MASK_VFMUL_VV)
DECLARE_INSN(vfmadd_vv, MATCH_VFMADD_VV, MASK_VFMADD_VV)
DECLARE_INSN(vfnmadd_vv, MATCH_VFNMADD_VV, MASK_VFNMADD_VV)
DECLARE_INSN(vfmsub_vv, MATCH_VFMSUB_VV, MASK_VFMSUB_VV)
DECLARE_INSN(vfnmsub_vv, MATCH_VFNMSUB_VV, MASK_VFNMSUB_VV)
DECLARE_INSN(vfmacc_vv, MATCH_VFMACC_VV, MASK_VFMACC_VV)
DECLARE_INSN(vfnmacc_vv, MATCH_VFNMACC_VV, MASK_VFNMACC_VV)
DECLARE_INSN(vfmsac_vv, MATCH_VFMSAC_VV, MASK_VFMSAC_VV)
DECLARE_INSN(vfnmsac_vv, MATCH_VFNMSAC_VV, MASK_VFNMSAC_VV)
DECLARE_INSN(vfcvt_xu_f_v, MATCH_VFCVT_XU_F_V, MASK_VFCVT_XU_F_V)
DECLARE_INSN(vfcvt_x_f_v, MATCH_VFCVT_X_F_V, MASK_VFCVT_X_F_V)
DECLARE_INSN(vfcvt_f_xu_v, MATCH_VFCVT_F_XU_V, MASK_VFCVT_F_XU_V)
DECLARE_INSN(vfcvt_f_x_v, MATCH_VFCVT_F_X_V, MASK_VFCVT_F_X_V)
DECLARE_INSN(vfcvt_rtz_xu_f_v, MATCH_VFCVT_RTZ_XU_F_V, MASK_VFCVT_RTZ_XU_F_V)
DECLARE_INSN(vfcvt_rtz_x_f_v, MATCH_VFCVT_RTZ_X_F_V, MASK_VFCVT_RTZ_X_F_V)
DECLARE_INSN(vfwcvt_xu_f_v, MATCH_VFWCVT_XU_F_V, MASK_VFWCVT_XU_F_V)
DECLARE_INSN(vfwcvt_x_f_v, MATCH_VFWCVT_X_F_V, MASK_VFWCVT_X_F_V)
DECLARE_INSN(vfwcvt_f_xu_v, MATCH_VFWCVT_F_XU_V, MASK_VFWCVT_F_XU_V)
DECLARE_INSN(vfwcvt_f_x_v, MATCH_VFWCVT_F_X_V, MASK_VFWCVT_F_X_V)
DECLARE_INSN(vfwcvt_f_f_v, MATCH_VFWCVT_F_F_V, MASK_VFWCVT_F_F_V)
DECLARE_INSN(vfwcvt_rtz_xu_f_v, MATCH_VFWCVT_RTZ_XU_F_V, MASK_VFWCVT_RTZ_XU_F_V)
DECLARE_INSN(vfwcvt_rtz_x_f_v, MATCH_VFWCVT_RTZ_X_F_V, MASK_VFWCVT_RTZ_X_F_V)
DECLARE_INSN(vfncvt_xu_f_w, MATCH_VFNCVT_XU_F_W, MASK_VFNCVT_XU_F_W)
DECLARE_INSN(vfncvt_x_f_w, MATCH_VFNCVT_X_F_W, MASK_VFNCVT_X_F_W)
DECLARE_INSN(vfncvt_f_xu_w, MATCH_VFNCVT_F_XU_W, MASK_VFNCVT_F_XU_W)
DECLARE_INSN(vfncvt_f_x_w, MATCH_VFNCVT_F_X_W, MASK_VFNCVT_F_X_W)
DECLARE_INSN(vfncvt_f_f_w, MATCH_VFNCVT_F_F_W, MASK_VFNCVT_F_F_W)
DECLARE_INSN(vfncvt_rod_f_f_w, MATCH_VFNCVT_ROD_F_F_W, MASK_VFNCVT_ROD_F_F_W)
DECLARE_INSN(vfncvt_rtz_xu_f_w, MATCH_VFNCVT_RTZ_XU_F_W, MASK_VFNCVT_RTZ_XU_F_W)
DECLARE_INSN(vfncvt_rtz_x_f_w, MATCH_VFNCVT_RTZ_X_F_W, MASK_VFNCVT_RTZ_X_F_W)
DECLARE_INSN(vfsqrt_v, MATCH_VFSQRT_V, MASK_VFSQRT_V)
DECLARE_INSN(vfrsqrte7_v, MATCH_VFRSQRTE7_V, MASK_VFRSQRTE7_V)
DECLARE_INSN(vfrece7_v, MATCH_VFRECE7_V, MASK_VFRECE7_V)
DECLARE_INSN(vfclass_v, MATCH_VFCLASS_V, MASK_VFCLASS_V)
DECLARE_INSN(vfwadd_vv, MATCH_VFWADD_VV, MASK_VFWADD_VV)
DECLARE_INSN(vfwredsum_vs, MATCH_VFWREDSUM_VS, MASK_VFWREDSUM_VS)
DECLARE_INSN(vfwsub_vv, MATCH_VFWSUB_VV, MASK_VFWSUB_VV)
DECLARE_INSN(vfwredosum_vs, MATCH_VFWREDOSUM_VS, MASK_VFWREDOSUM_VS)
DECLARE_INSN(vfwadd_wv, MATCH_VFWADD_WV, MASK_VFWADD_WV)
DECLARE_INSN(vfwsub_wv, MATCH_VFWSUB_WV, MASK_VFWSUB_WV)
DECLARE_INSN(vfwmul_vv, MATCH_VFWMUL_VV, MASK_VFWMUL_VV)
DECLARE_INSN(vfdot_vv, MATCH_VFDOT_VV, MASK_VFDOT_VV)
DECLARE_INSN(vfwmacc_vv, MATCH_VFWMACC_VV, MASK_VFWMACC_VV)
DECLARE_INSN(vfwnmacc_vv, MATCH_VFWNMACC_VV, MASK_VFWNMACC_VV)
DECLARE_INSN(vfwmsac_vv, MATCH_VFWMSAC_VV, MASK_VFWMSAC_VV)
DECLARE_INSN(vfwnmsac_vv, MATCH_VFWNMSAC_VV, MASK_VFWNMSAC_VV)
DECLARE_INSN(vadd_vx, MATCH_VADD_VX, MASK_VADD_VX)
DECLARE_INSN(vsub_vx, MATCH_VSUB_VX, MASK_VSUB_VX)
DECLARE_INSN(vrsub_vx, MATCH_VRSUB_VX, MASK_VRSUB_VX)
DECLARE_INSN(vminu_vx, MATCH_VMINU_VX, MASK_VMINU_VX)
DECLARE_INSN(vmin_vx, MATCH_VMIN_VX, MASK_VMIN_VX)
DECLARE_INSN(vmaxu_vx, MATCH_VMAXU_VX, MASK_VMAXU_VX)
DECLARE_INSN(vmax_vx, MATCH_VMAX_VX, MASK_VMAX_VX)
DECLARE_INSN(vand_vx, MATCH_VAND_VX, MASK_VAND_VX)
DECLARE_INSN(vor_vx, MATCH_VOR_VX, MASK_VOR_VX)
DECLARE_INSN(vxor_vx, MATCH_VXOR_VX, MASK_VXOR_VX)
DECLARE_INSN(vrgather_vx, MATCH_VRGATHER_VX, MASK_VRGATHER_VX)
DECLARE_INSN(vslideup_vx, MATCH_VSLIDEUP_VX, MASK_VSLIDEUP_VX)
DECLARE_INSN(vslidedown_vx, MATCH_VSLIDEDOWN_VX, MASK_VSLIDEDOWN_VX)
DECLARE_INSN(vadc_vxm, MATCH_VADC_VXM, MASK_VADC_VXM)
DECLARE_INSN(vmadc_vxm, MATCH_VMADC_VXM, MASK_VMADC_VXM)
DECLARE_INSN(vsbc_vxm, MATCH_VSBC_VXM, MASK_VSBC_VXM)
DECLARE_INSN(vmsbc_vxm, MATCH_VMSBC_VXM, MASK_VMSBC_VXM)
DECLARE_INSN(vmerge_vxm, MATCH_VMERGE_VXM, MASK_VMERGE_VXM)
DECLARE_INSN(vmv_v_x, MATCH_VMV_V_X, MASK_VMV_V_X)
DECLARE_INSN(vmseq_vx, MATCH_VMSEQ_VX, MASK_VMSEQ_VX)
DECLARE_INSN(vmsne_vx, MATCH_VMSNE_VX, MASK_VMSNE_VX)
DECLARE_INSN(vmsltu_vx, MATCH_VMSLTU_VX, MASK_VMSLTU_VX)
DECLARE_INSN(vmslt_vx, MATCH_VMSLT_VX, MASK_VMSLT_VX)
DECLARE_INSN(vmsleu_vx, MATCH_VMSLEU_VX, MASK_VMSLEU_VX)
DECLARE_INSN(vmsle_vx, MATCH_VMSLE_VX, MASK_VMSLE_VX)
DECLARE_INSN(vmsgtu_vx, MATCH_VMSGTU_VX, MASK_VMSGTU_VX)
DECLARE_INSN(vmsgt_vx, MATCH_VMSGT_VX, MASK_VMSGT_VX)
DECLARE_INSN(vsaddu_vx, MATCH_VSADDU_VX, MASK_VSADDU_VX)
DECLARE_INSN(vsadd_vx, MATCH_VSADD_VX, MASK_VSADD_VX)
DECLARE_INSN(vssubu_vx, MATCH_VSSUBU_VX, MASK_VSSUBU_VX)
DECLARE_INSN(vssub_vx, MATCH_VSSUB_VX, MASK_VSSUB_VX)
DECLARE_INSN(vsll_vx, MATCH_VSLL_VX, MASK_VSLL_VX)
DECLARE_INSN(vsmul_vx, MATCH_VSMUL_VX, MASK_VSMUL_VX)
DECLARE_INSN(vsrl_vx, MATCH_VSRL_VX, MASK_VSRL_VX)
DECLARE_INSN(vsra_vx, MATCH_VSRA_VX, MASK_VSRA_VX)
DECLARE_INSN(vssrl_vx, MATCH_VSSRL_VX, MASK_VSSRL_VX)
DECLARE_INSN(vssra_vx, MATCH_VSSRA_VX, MASK_VSSRA_VX)
DECLARE_INSN(vnsrl_wx, MATCH_VNSRL_WX, MASK_VNSRL_WX)
DECLARE_INSN(vnsra_wx, MATCH_VNSRA_WX, MASK_VNSRA_WX)
DECLARE_INSN(vnclipu_wx, MATCH_VNCLIPU_WX, MASK_VNCLIPU_WX)
DECLARE_INSN(vnclip_wx, MATCH_VNCLIP_WX, MASK_VNCLIP_WX)
DECLARE_INSN(vqmaccu_vx, MATCH_VQMACCU_VX, MASK_VQMACCU_VX)
DECLARE_INSN(vqmacc_vx, MATCH_VQMACC_VX, MASK_VQMACC_VX)
DECLARE_INSN(vqmaccus_vx, MATCH_VQMACCUS_VX, MASK_VQMACCUS_VX)
DECLARE_INSN(vqmaccsu_vx, MATCH_VQMACCSU_VX, MASK_VQMACCSU_VX)
DECLARE_INSN(vadd_vv, MATCH_VADD_VV, MASK_VADD_VV)
DECLARE_INSN(vsub_vv, MATCH_VSUB_VV, MASK_VSUB_VV)
DECLARE_INSN(vminu_vv, MATCH_VMINU_VV, MASK_VMINU_VV)
DECLARE_INSN(vmin_vv, MATCH_VMIN_VV, MASK_VMIN_VV)
DECLARE_INSN(vmaxu_vv, MATCH_VMAXU_VV, MASK_VMAXU_VV)
DECLARE_INSN(vmax_vv, MATCH_VMAX_VV, MASK_VMAX_VV)
DECLARE_INSN(vand_vv, MATCH_VAND_VV, MASK_VAND_VV)
DECLARE_INSN(vor_vv, MATCH_VOR_VV, MASK_VOR_VV)
DECLARE_INSN(vxor_vv, MATCH_VXOR_VV, MASK_VXOR_VV)
DECLARE_INSN(vrgather_vv, MATCH_VRGATHER_VV, MASK_VRGATHER_VV)
DECLARE_INSN(vrgatherei16_vv, MATCH_VRGATHEREI16_VV, MASK_VRGATHEREI16_VV)
DECLARE_INSN(vadc_vvm, MATCH_VADC_VVM, MASK_VADC_VVM)
DECLARE_INSN(vmadc_vvm, MATCH_VMADC_VVM, MASK_VMADC_VVM)
DECLARE_INSN(vsbc_vvm, MATCH_VSBC_VVM, MASK_VSBC_VVM)
DECLARE_INSN(vmsbc_vvm, MATCH_VMSBC_VVM, MASK_VMSBC_VVM)
DECLARE_INSN(vmerge_vvm, MATCH_VMERGE_VVM, MASK_VMERGE_VVM)
DECLARE_INSN(vmv_v_v, MATCH_VMV_V_V, MASK_VMV_V_V)
DECLARE_INSN(vmseq_vv, MATCH_VMSEQ_VV, MASK_VMSEQ_VV)
DECLARE_INSN(vmsne_vv, MATCH_VMSNE_VV, MASK_VMSNE_VV)
DECLARE_INSN(vmsltu_vv, MATCH_VMSLTU_VV, MASK_VMSLTU_VV)
DECLARE_INSN(vmslt_vv, MATCH_VMSLT_VV, MASK_VMSLT_VV)
DECLARE_INSN(vmsleu_vv, MATCH_VMSLEU_VV, MASK_VMSLEU_VV)
DECLARE_INSN(vmsle_vv, MATCH_VMSLE_VV, MASK_VMSLE_VV)
DECLARE_INSN(vsaddu_vv, MATCH_VSADDU_VV, MASK_VSADDU_VV)
DECLARE_INSN(vsadd_vv, MATCH_VSADD_VV, MASK_VSADD_VV)
DECLARE_INSN(vssubu_vv, MATCH_VSSUBU_VV, MASK_VSSUBU_VV)
DECLARE_INSN(vssub_vv, MATCH_VSSUB_VV, MASK_VSSUB_VV)
DECLARE_INSN(vsll_vv, MATCH_VSLL_VV, MASK_VSLL_VV)
DECLARE_INSN(vsmul_vv, MATCH_VSMUL_VV, MASK_VSMUL_VV)
DECLARE_INSN(vsrl_vv, MATCH_VSRL_VV, MASK_VSRL_VV)
DECLARE_INSN(vsra_vv, MATCH_VSRA_VV, MASK_VSRA_VV)
DECLARE_INSN(vssrl_vv, MATCH_VSSRL_VV, MASK_VSSRL_VV)
DECLARE_INSN(vssra_vv, MATCH_VSSRA_VV, MASK_VSSRA_VV)
DECLARE_INSN(vnsrl_wv, MATCH_VNSRL_WV, MASK_VNSRL_WV)
DECLARE_INSN(vnsra_wv, MATCH_VNSRA_WV, MASK_VNSRA_WV)
DECLARE_INSN(vnclipu_wv, MATCH_VNCLIPU_WV, MASK_VNCLIPU_WV)
DECLARE_INSN(vnclip_wv, MATCH_VNCLIP_WV, MASK_VNCLIP_WV)
DECLARE_INSN(vwredsumu_vs, MATCH_VWREDSUMU_VS, MASK_VWREDSUMU_VS)
DECLARE_INSN(vwredsum_vs, MATCH_VWREDSUM_VS, MASK_VWREDSUM_VS)
DECLARE_INSN(vdotu_vv, MATCH_VDOTU_VV, MASK_VDOTU_VV)
DECLARE_INSN(vdot_vv, MATCH_VDOT_VV, MASK_VDOT_VV)
DECLARE_INSN(vqmaccu_vv, MATCH_VQMACCU_VV, MASK_VQMACCU_VV)
DECLARE_INSN(vqmacc_vv, MATCH_VQMACC_VV, MASK_VQMACC_VV)
DECLARE_INSN(vqmaccsu_vv, MATCH_VQMACCSU_VV, MASK_VQMACCSU_VV)
DECLARE_INSN(vadd_vi, MATCH_VADD_VI, MASK_VADD_VI)
DECLARE_INSN(vrsub_vi, MATCH_VRSUB_VI, MASK_VRSUB_VI)
DECLARE_INSN(vand_vi, MATCH_VAND_VI, MASK_VAND_VI)
DECLARE_INSN(vor_vi, MATCH_VOR_VI, MASK_VOR_VI)
DECLARE_INSN(vxor_vi, MATCH_VXOR_VI, MASK_VXOR_VI)
DECLARE_INSN(vrgather_vi, MATCH_VRGATHER_VI, MASK_VRGATHER_VI)
DECLARE_INSN(vslideup_vi, MATCH_VSLIDEUP_VI, MASK_VSLIDEUP_VI)
DECLARE_INSN(vslidedown_vi, MATCH_VSLIDEDOWN_VI, MASK_VSLIDEDOWN_VI)
DECLARE_INSN(vadc_vim, MATCH_VADC_VIM, MASK_VADC_VIM)
DECLARE_INSN(vmadc_vim, MATCH_VMADC_VIM, MASK_VMADC_VIM)
DECLARE_INSN(vmerge_vim, MATCH_VMERGE_VIM, MASK_VMERGE_VIM)
DECLARE_INSN(vmv_v_i, MATCH_VMV_V_I, MASK_VMV_V_I)
DECLARE_INSN(vmseq_vi, MATCH_VMSEQ_VI, MASK_VMSEQ_VI)
DECLARE_INSN(vmsne_vi, MATCH_VMSNE_VI, MASK_VMSNE_VI)
DECLARE_INSN(vmsleu_vi, MATCH_VMSLEU_VI, MASK_VMSLEU_VI)
DECLARE_INSN(vmsle_vi, MATCH_VMSLE_VI, MASK_VMSLE_VI)
DECLARE_INSN(vmsgtu_vi, MATCH_VMSGTU_VI, MASK_VMSGTU_VI)
DECLARE_INSN(vmsgt_vi, MATCH_VMSGT_VI, MASK_VMSGT_VI)
DECLARE_INSN(vsaddu_vi, MATCH_VSADDU_VI, MASK_VSADDU_VI)
DECLARE_INSN(vsadd_vi, MATCH_VSADD_VI, MASK_VSADD_VI)
DECLARE_INSN(vsll_vi, MATCH_VSLL_VI, MASK_VSLL_VI)
DECLARE_INSN(vmv1r_v, MATCH_VMV1R_V, MASK_VMV1R_V)
DECLARE_INSN(vmv2r_v, MATCH_VMV2R_V, MASK_VMV2R_V)
DECLARE_INSN(vmv4r_v, MATCH_VMV4R_V, MASK_VMV4R_V)
DECLARE_INSN(vmv8r_v, MATCH_VMV8R_V, MASK_VMV8R_V)
DECLARE_INSN(vsrl_vi, MATCH_VSRL_VI, MASK_VSRL_VI)
DECLARE_INSN(vsra_vi, MATCH_VSRA_VI, MASK_VSRA_VI)
DECLARE_INSN(vssrl_vi, MATCH_VSSRL_VI, MASK_VSSRL_VI)
DECLARE_INSN(vssra_vi, MATCH_VSSRA_VI, MASK_VSSRA_VI)
DECLARE_INSN(vnsrl_wi, MATCH_VNSRL_WI, MASK_VNSRL_WI)
DECLARE_INSN(vnsra_wi, MATCH_VNSRA_WI, MASK_VNSRA_WI)
DECLARE_INSN(vnclipu_wi, MATCH_VNCLIPU_WI, MASK_VNCLIPU_WI)
DECLARE_INSN(vnclip_wi, MATCH_VNCLIP_WI, MASK_VNCLIP_WI)
DECLARE_INSN(vredsum_vs, MATCH_VREDSUM_VS, MASK_VREDSUM_VS)
DECLARE_INSN(vredand_vs, MATCH_VREDAND_VS, MASK_VREDAND_VS)
DECLARE_INSN(vredor_vs, MATCH_VREDOR_VS, MASK_VREDOR_VS)
DECLARE_INSN(vredxor_vs, MATCH_VREDXOR_VS, MASK_VREDXOR_VS)
DECLARE_INSN(vredminu_vs, MATCH_VREDMINU_VS, MASK_VREDMINU_VS)
DECLARE_INSN(vredmin_vs, MATCH_VREDMIN_VS, MASK_VREDMIN_VS)
DECLARE_INSN(vredmaxu_vs, MATCH_VREDMAXU_VS, MASK_VREDMAXU_VS)
DECLARE_INSN(vredmax_vs, MATCH_VREDMAX_VS, MASK_VREDMAX_VS)
DECLARE_INSN(vaaddu_vv, MATCH_VAADDU_VV, MASK_VAADDU_VV)
DECLARE_INSN(vaadd_vv, MATCH_VAADD_VV, MASK_VAADD_VV)
DECLARE_INSN(vasubu_vv, MATCH_VASUBU_VV, MASK_VASUBU_VV)
DECLARE_INSN(vasub_vv, MATCH_VASUB_VV, MASK_VASUB_VV)
DECLARE_INSN(vmv_x_s, MATCH_VMV_X_S, MASK_VMV_X_S)
DECLARE_INSN(vzext_vf8, MATCH_VZEXT_VF8, MASK_VZEXT_VF8)
DECLARE_INSN(vsext_vf8, MATCH_VSEXT_VF8, MASK_VSEXT_VF8)
DECLARE_INSN(vzext_vf4, MATCH_VZEXT_VF4, MASK_VZEXT_VF4)
DECLARE_INSN(vsext_vf4, MATCH_VSEXT_VF4, MASK_VSEXT_VF4)
DECLARE_INSN(vzext_vf2, MATCH_VZEXT_VF2, MASK_VZEXT_VF2)
DECLARE_INSN(vsext_vf2, MATCH_VSEXT_VF2, MASK_VSEXT_VF2)
DECLARE_INSN(vcompress_vm, MATCH_VCOMPRESS_VM, MASK_VCOMPRESS_VM)
DECLARE_INSN(vmandnot_mm, MATCH_VMANDNOT_MM, MASK_VMANDNOT_MM)
DECLARE_INSN(vmand_mm, MATCH_VMAND_MM, MASK_VMAND_MM)
DECLARE_INSN(vmor_mm, MATCH_VMOR_MM, MASK_VMOR_MM)
DECLARE_INSN(vmxor_mm, MATCH_VMXOR_MM, MASK_VMXOR_MM)
DECLARE_INSN(vmornot_mm, MATCH_VMORNOT_MM, MASK_VMORNOT_MM)
DECLARE_INSN(vmnand_mm, MATCH_VMNAND_MM, MASK_VMNAND_MM)
DECLARE_INSN(vmnor_mm, MATCH_VMNOR_MM, MASK_VMNOR_MM)
DECLARE_INSN(vmxnor_mm, MATCH_VMXNOR_MM, MASK_VMXNOR_MM)
DECLARE_INSN(vmsbf_m, MATCH_VMSBF_M, MASK_VMSBF_M)
DECLARE_INSN(vmsof_m, MATCH_VMSOF_M, MASK_VMSOF_M)
DECLARE_INSN(vmsif_m, MATCH_VMSIF_M, MASK_VMSIF_M)
DECLARE_INSN(viota_m, MATCH_VIOTA_M, MASK_VIOTA_M)
DECLARE_INSN(vid_v, MATCH_VID_V, MASK_VID_V)
DECLARE_INSN(vpopc_m, MATCH_VPOPC_M, MASK_VPOPC_M)
DECLARE_INSN(vfirst_m, MATCH_VFIRST_M, MASK_VFIRST_M)
DECLARE_INSN(vdivu_vv, MATCH_VDIVU_VV, MASK_VDIVU_VV)
DECLARE_INSN(vdiv_vv, MATCH_VDIV_VV, MASK_VDIV_VV)
DECLARE_INSN(vremu_vv, MATCH_VREMU_VV, MASK_VREMU_VV)
DECLARE_INSN(vrem_vv, MATCH_VREM_VV, MASK_VREM_VV)
DECLARE_INSN(vmulhu_vv, MATCH_VMULHU_VV, MASK_VMULHU_VV)
DECLARE_INSN(vmul_vv, MATCH_VMUL_VV, MASK_VMUL_VV)
DECLARE_INSN(vmulhsu_vv, MATCH_VMULHSU_VV, MASK_VMULHSU_VV)
DECLARE_INSN(vmulh_vv, MATCH_VMULH_VV, MASK_VMULH_VV)
DECLARE_INSN(vmadd_vv, MATCH_VMADD_VV, MASK_VMADD_VV)
DECLARE_INSN(vnmsub_vv, MATCH_VNMSUB_VV, MASK_VNMSUB_VV)
DECLARE_INSN(vmacc_vv, MATCH_VMACC_VV, MASK_VMACC_VV)
DECLARE_INSN(vnmsac_vv, MATCH_VNMSAC_VV, MASK_VNMSAC_VV)
DECLARE_INSN(vwaddu_vv, MATCH_VWADDU_VV, MASK_VWADDU_VV)
DECLARE_INSN(vwadd_vv, MATCH_VWADD_VV, MASK_VWADD_VV)
DECLARE_INSN(vwsubu_vv, MATCH_VWSUBU_VV, MASK_VWSUBU_VV)
DECLARE_INSN(vwsub_vv, MATCH_VWSUB_VV, MASK_VWSUB_VV)
DECLARE_INSN(vwaddu_wv, MATCH_VWADDU_WV, MASK_VWADDU_WV)
DECLARE_INSN(vwadd_wv, MATCH_VWADD_WV, MASK_VWADD_WV)
DECLARE_INSN(vwsubu_wv, MATCH_VWSUBU_WV, MASK_VWSUBU_WV)
DECLARE_INSN(vwsub_wv, MATCH_VWSUB_WV, MASK_VWSUB_WV)
DECLARE_INSN(vwmulu_vv, MATCH_VWMULU_VV, MASK_VWMULU_VV)
DECLARE_INSN(vwmulsu_vv, MATCH_VWMULSU_VV, MASK_VWMULSU_VV)
DECLARE_INSN(vwmul_vv, MATCH_VWMUL_VV, MASK_VWMUL_VV)
DECLARE_INSN(vwmaccu_vv, MATCH_VWMACCU_VV, MASK_VWMACCU_VV)
DECLARE_INSN(vwmacc_vv, MATCH_VWMACC_VV, MASK_VWMACC_VV)
DECLARE_INSN(vwmaccsu_vv, MATCH_VWMACCSU_VV, MASK_VWMACCSU_VV)
DECLARE_INSN(vaaddu_vx, MATCH_VAADDU_VX, MASK_VAADDU_VX)
DECLARE_INSN(vaadd_vx, MATCH_VAADD_VX, MASK_VAADD_VX)
DECLARE_INSN(vasubu_vx, MATCH_VASUBU_VX, MASK_VASUBU_VX)
DECLARE_INSN(vasub_vx, MATCH_VASUB_VX, MASK_VASUB_VX)
DECLARE_INSN(vmv_s_x, MATCH_VMV_S_X, MASK_VMV_S_X)
DECLARE_INSN(vslide1up_vx, MATCH_VSLIDE1UP_VX, MASK_VSLIDE1UP_VX)
DECLARE_INSN(vslide1down_vx, MATCH_VSLIDE1DOWN_VX, MASK_VSLIDE1DOWN_VX)
DECLARE_INSN(vdivu_vx, MATCH_VDIVU_VX, MASK_VDIVU_VX)
DECLARE_INSN(vdiv_vx, MATCH_VDIV_VX, MASK_VDIV_VX)
DECLARE_INSN(vremu_vx, MATCH_VREMU_VX, MASK_VREMU_VX)
DECLARE_INSN(vrem_vx, MATCH_VREM_VX, MASK_VREM_VX)
DECLARE_INSN(vmulhu_vx, MATCH_VMULHU_VX, MASK_VMULHU_VX)
DECLARE_INSN(vmul_vx, MATCH_VMUL_VX, MASK_VMUL_VX)
DECLARE_INSN(vmulhsu_vx, MATCH_VMULHSU_VX, MASK_VMULHSU_VX)
DECLARE_INSN(vmulh_vx, MATCH_VMULH_VX, MASK_VMULH_VX)
DECLARE_INSN(vmadd_vx, MATCH_VMADD_VX, MASK_VMADD_VX)
DECLARE_INSN(vnmsub_vx, MATCH_VNMSUB_VX, MASK_VNMSUB_VX)
DECLARE_INSN(vmacc_vx, MATCH_VMACC_VX, MASK_VMACC_VX)
DECLARE_INSN(vnmsac_vx, MATCH_VNMSAC_VX, MASK_VNMSAC_VX)
DECLARE_INSN(vwaddu_vx, MATCH_VWADDU_VX, MASK_VWADDU_VX)
DECLARE_INSN(vwadd_vx, MATCH_VWADD_VX, MASK_VWADD_VX)
DECLARE_INSN(vwsubu_vx, MATCH_VWSUBU_VX, MASK_VWSUBU_VX)
DECLARE_INSN(vwsub_vx, MATCH_VWSUB_VX, MASK_VWSUB_VX)
DECLARE_INSN(vwaddu_wx, MATCH_VWADDU_WX, MASK_VWADDU_WX)
DECLARE_INSN(vwadd_wx, MATCH_VWADD_WX, MASK_VWADD_WX)
DECLARE_INSN(vwsubu_wx, MATCH_VWSUBU_WX, MASK_VWSUBU_WX)
DECLARE_INSN(vwsub_wx, MATCH_VWSUB_WX, MASK_VWSUB_WX)
DECLARE_INSN(vwmulu_vx, MATCH_VWMULU_VX, MASK_VWMULU_VX)
DECLARE_INSN(vwmulsu_vx, MATCH_VWMULSU_VX, MASK_VWMULSU_VX)
DECLARE_INSN(vwmul_vx, MATCH_VWMUL_VX, MASK_VWMUL_VX)
DECLARE_INSN(vwmaccu_vx, MATCH_VWMACCU_VX, MASK_VWMACCU_VX)
DECLARE_INSN(vwmacc_vx, MATCH_VWMACC_VX, MASK_VWMACC_VX)
DECLARE_INSN(vwmaccus_vx, MATCH_VWMACCUS_VX, MASK_VWMACCUS_VX)
DECLARE_INSN(vwmaccsu_vx, MATCH_VWMACCSU_VX, MASK_VWMACCSU_VX)
DECLARE_INSN(vamoswapei8_v, MATCH_VAMOSWAPEI8_V, MASK_VAMOSWAPEI8_V)
DECLARE_INSN(vamoaddei8_v, MATCH_VAMOADDEI8_V, MASK_VAMOADDEI8_V)
DECLARE_INSN(vamoxorei8_v, MATCH_VAMOXOREI8_V, MASK_VAMOXOREI8_V)
DECLARE_INSN(vamoandei8_v, MATCH_VAMOANDEI8_V, MASK_VAMOANDEI8_V)
DECLARE_INSN(vamoorei8_v, MATCH_VAMOOREI8_V, MASK_VAMOOREI8_V)
DECLARE_INSN(vamominei8_v, MATCH_VAMOMINEI8_V, MASK_VAMOMINEI8_V)
DECLARE_INSN(vamomaxei8_v, MATCH_VAMOMAXEI8_V, MASK_VAMOMAXEI8_V)
DECLARE_INSN(vamominuei8_v, MATCH_VAMOMINUEI8_V, MASK_VAMOMINUEI8_V)
DECLARE_INSN(vamomaxuei8_v, MATCH_VAMOMAXUEI8_V, MASK_VAMOMAXUEI8_V)
DECLARE_INSN(vamoswapei16_v, MATCH_VAMOSWAPEI16_V, MASK_VAMOSWAPEI16_V)
DECLARE_INSN(vamoaddei16_v, MATCH_VAMOADDEI16_V, MASK_VAMOADDEI16_V)
DECLARE_INSN(vamoxorei16_v, MATCH_VAMOXOREI16_V, MASK_VAMOXOREI16_V)
DECLARE_INSN(vamoandei16_v, MATCH_VAMOANDEI16_V, MASK_VAMOANDEI16_V)
DECLARE_INSN(vamoorei16_v, MATCH_VAMOOREI16_V, MASK_VAMOOREI16_V)
DECLARE_INSN(vamominei16_v, MATCH_VAMOMINEI16_V, MASK_VAMOMINEI16_V)
DECLARE_INSN(vamomaxei16_v, MATCH_VAMOMAXEI16_V, MASK_VAMOMAXEI16_V)
DECLARE_INSN(vamominuei16_v, MATCH_VAMOMINUEI16_V, MASK_VAMOMINUEI16_V)
DECLARE_INSN(vamomaxuei16_v, MATCH_VAMOMAXUEI16_V, MASK_VAMOMAXUEI16_V)
DECLARE_INSN(vamoswapei32_v, MATCH_VAMOSWAPEI32_V, MASK_VAMOSWAPEI32_V)
DECLARE_INSN(vamoaddei32_v, MATCH_VAMOADDEI32_V, MASK_VAMOADDEI32_V)
DECLARE_INSN(vamoxorei32_v, MATCH_VAMOXOREI32_V, MASK_VAMOXOREI32_V)
DECLARE_INSN(vamoandei32_v, MATCH_VAMOANDEI32_V, MASK_VAMOANDEI32_V)
DECLARE_INSN(vamoorei32_v, MATCH_VAMOOREI32_V, MASK_VAMOOREI32_V)
DECLARE_INSN(vamominei32_v, MATCH_VAMOMINEI32_V, MASK_VAMOMINEI32_V)
DECLARE_INSN(vamomaxei32_v, MATCH_VAMOMAXEI32_V, MASK_VAMOMAXEI32_V)
DECLARE_INSN(vamominuei32_v, MATCH_VAMOMINUEI32_V, MASK_VAMOMINUEI32_V)
DECLARE_INSN(vamomaxuei32_v, MATCH_VAMOMAXUEI32_V, MASK_VAMOMAXUEI32_V)
DECLARE_INSN(vamoswapei64_v, MATCH_VAMOSWAPEI64_V, MASK_VAMOSWAPEI64_V)
DECLARE_INSN(vamoaddei64_v, MATCH_VAMOADDEI64_V, MASK_VAMOADDEI64_V)
DECLARE_INSN(vamoxorei64_v, MATCH_VAMOXOREI64_V, MASK_VAMOXOREI64_V)
DECLARE_INSN(vamoandei64_v, MATCH_VAMOANDEI64_V, MASK_VAMOANDEI64_V)
DECLARE_INSN(vamoorei64_v, MATCH_VAMOOREI64_V, MASK_VAMOOREI64_V)
DECLARE_INSN(vamominei64_v, MATCH_VAMOMINEI64_V, MASK_VAMOMINEI64_V)
DECLARE_INSN(vamomaxei64_v, MATCH_VAMOMAXEI64_V, MASK_VAMOMAXEI64_V)
DECLARE_INSN(vamominuei64_v, MATCH_VAMOMINUEI64_V, MASK_VAMOMINUEI64_V)
DECLARE_INSN(vamomaxuei64_v, MATCH_VAMOMAXUEI64_V, MASK_VAMOMAXUEI64_V)
DECLARE_INSN(vmvnfr_v, MATCH_VMVNFR_V, MASK_VMVNFR_V)
DECLARE_INSN(vl1r_v, MATCH_VL1R_V, MASK_VL1R_V)
DECLARE_INSN(vl2r_v, MATCH_VL2R_V, MASK_VL2R_V)
DECLARE_INSN(vl4r_v, MATCH_VL4R_V, MASK_VL4R_V)
DECLARE_INSN(vl8r_v, MATCH_VL8R_V, MASK_VL8R_V)
#endif
#ifdef DECLARE_CSR
DECLARE_CSR(fflags, CSR_FFLAGS)
DECLARE_CSR(frm, CSR_FRM)
DECLARE_CSR(fcsr, CSR_FCSR)
DECLARE_CSR(ustatus, CSR_USTATUS)
DECLARE_CSR(uie, CSR_UIE)
DECLARE_CSR(utvec, CSR_UTVEC)
DECLARE_CSR(vstart, CSR_VSTART)
DECLARE_CSR(vxsat, CSR_VXSAT)
DECLARE_CSR(vxrm, CSR_VXRM)
DECLARE_CSR(vcsr, CSR_VCSR)
DECLARE_CSR(uscratch, CSR_USCRATCH)
DECLARE_CSR(uepc, CSR_UEPC)
DECLARE_CSR(ucause, CSR_UCAUSE)
DECLARE_CSR(utval, CSR_UTVAL)
DECLARE_CSR(uip, CSR_UIP)
DECLARE_CSR(cycle, CSR_CYCLE)
DECLARE_CSR(time, CSR_TIME)
DECLARE_CSR(instret, CSR_INSTRET)
DECLARE_CSR(hpmcounter3, CSR_HPMCOUNTER3)
DECLARE_CSR(hpmcounter4, CSR_HPMCOUNTER4)
DECLARE_CSR(hpmcounter5, CSR_HPMCOUNTER5)
DECLARE_CSR(hpmcounter6, CSR_HPMCOUNTER6)
DECLARE_CSR(hpmcounter7, CSR_HPMCOUNTER7)
DECLARE_CSR(hpmcounter8, CSR_HPMCOUNTER8)
DECLARE_CSR(hpmcounter9, CSR_HPMCOUNTER9)
DECLARE_CSR(hpmcounter10, CSR_HPMCOUNTER10)
DECLARE_CSR(hpmcounter11, CSR_HPMCOUNTER11)
DECLARE_CSR(hpmcounter12, CSR_HPMCOUNTER12)
DECLARE_CSR(hpmcounter13, CSR_HPMCOUNTER13)
DECLARE_CSR(hpmcounter14, CSR_HPMCOUNTER14)
DECLARE_CSR(hpmcounter15, CSR_HPMCOUNTER15)
DECLARE_CSR(hpmcounter16, CSR_HPMCOUNTER16)
DECLARE_CSR(hpmcounter17, CSR_HPMCOUNTER17)
DECLARE_CSR(hpmcounter18, CSR_HPMCOUNTER18)
DECLARE_CSR(hpmcounter19, CSR_HPMCOUNTER19)
DECLARE_CSR(hpmcounter20, CSR_HPMCOUNTER20)
DECLARE_CSR(hpmcounter21, CSR_HPMCOUNTER21)
DECLARE_CSR(hpmcounter22, CSR_HPMCOUNTER22)
DECLARE_CSR(hpmcounter23, CSR_HPMCOUNTER23)
DECLARE_CSR(hpmcounter24, CSR_HPMCOUNTER24)
DECLARE_CSR(hpmcounter25, CSR_HPMCOUNTER25)
DECLARE_CSR(hpmcounter26, CSR_HPMCOUNTER26)
DECLARE_CSR(hpmcounter27, CSR_HPMCOUNTER27)
DECLARE_CSR(hpmcounter28, CSR_HPMCOUNTER28)
DECLARE_CSR(hpmcounter29, CSR_HPMCOUNTER29)
DECLARE_CSR(hpmcounter30, CSR_HPMCOUNTER30)
DECLARE_CSR(hpmcounter31, CSR_HPMCOUNTER31)
DECLARE_CSR(vl, CSR_VL)
DECLARE_CSR(vtype, CSR_VTYPE)
DECLARE_CSR(vlenb, CSR_VLENB)
DECLARE_CSR(vme_shape_s, CSR_VME_SHAPE_S)
DECLARE_CSR(vme_stride_d, CSR_VME_STRIDE_D)
DECLARE_CSR(vme_stride_s, CSR_VME_STRIDE_S)
DECLARE_CSR(mme_shape_s1, CSR_MME_SHAPE_S1)
DECLARE_CSR(mme_shape_s2, CSR_MME_SHAPE_S2)
DECLARE_CSR(mme_stride_d, CSR_MME_STRIDE_D)
DECLARE_CSR(mme_stride_s, CSR_MME_STRIDE_S)
DECLARE_CSR(conv_FM_in, CSR_CONV_FM_IN)
DECLARE_CSR(conv_Cin, CSR_CONV_CIN)
DECLARE_CSR(conv_FM_out, CSR_CONV_FM_OUT)
DECLARE_CSR(conv_Cout, CSR_CONV_COUT)
DECLARE_CSR(conv_kernel_params2, CSR_CONV_KERNEL_PARAMS2)
DECLARE_CSR(conv_kernel_params1, CSR_CONV_KERNEL_PARAMS1)
DECLARE_CSR(conv_padding, CSR_CONV_PADDING)
DECLARE_CSR(mme_dequant_coeff, CSR_MME_DEQUANT_COEFF)
DECLARE_CSR(mme_quant_coeff, CSR_MME_QUANT_COEFF)
DECLARE_CSR(mme_sparseidx_base, CSR_MME_SPARSEIDX_BASE)
DECLARE_CSR(mme_sparseidx_stride, CSR_MME_SPARSEIDX_STRIDE)
DECLARE_CSR(vme_data_type, CSR_VME_DATA_TYPE)
DECLARE_CSR(mme_data_type, CSR_MME_DATA_TYPE)
DECLARE_CSR(vme_relu_threshhold, CSR_VME_RELU_THRESHHOLD)
DECLARE_CSR(vme_FM_in, CSR_VME_FM_IN)
DECLARE_CSR(vme_Cin, CSR_VME_CIN)
DECLARE_CSR(vme_FM_out, CSR_VME_FM_OUT)
DECLARE_CSR(vme_Cout, CSR_VME_COUT)
DECLARE_CSR(vme_kernel_param1, CSR_VME_KERNEL_PARAM1)
DECLARE_CSR(vme_FM_padding, CSR_VME_FM_PADDING)
DECLARE_CSR(vme_kernel_param2, CSR_VME_KERNEL_PARAM2)
DECLARE_CSR(vme_max_min_idx, CSR_VME_MAX_MIN_IDX)
DECLARE_CSR(ncp_busy, CSR_NCP_BUSY)
DECLARE_CSR(mte_stride_s, CSR_MTE_STRIDE_S)
DECLARE_CSR(mte_icdest, CSR_MTE_ICDEST)
DECLARE_CSR(mte_shape, CSR_MTE_SHAPE)
DECLARE_CSR(mte_stride_d, CSR_MTE_STRIDE_D)
DECLARE_CSR(tid, CSR_TID)
DECLARE_CSR(tmisc, CSR_TMISC)
DECLARE_CSR(tcsr, CSR_TCSR)
DECLARE_CSR(mte_data_type, CSR_MTE_DATA_TYPE)
DECLARE_CSR(dmae_data_type, CSR_DMAE_DATA_TYPE)
DECLARE_CSR(dmae_shape_1, CSR_DMAE_SHAPE_1)
DECLARE_CSR(dmae_shape_2, CSR_DMAE_SHAPE_2)
DECLARE_CSR(dmae_stride_s1, CSR_DMAE_STRIDE_S1)
DECLARE_CSR(dmae_stride_s2, CSR_DMAE_STRIDE_S2)
DECLARE_CSR(dmae_stride_d1, CSR_DMAE_STRIDE_D1)
DECLARE_CSR(dmae_stride_d2, CSR_DMAE_STRIDE_D2)
DECLARE_CSR(dmae_ctrl, CSR_DMAE_CTRL)
DECLARE_CSR(mdcm_cfg, CSR_MDCM_CFG)
DECLARE_CSR(micm_cfg, CSR_MICM_CFG)
DECLARE_CSR(mcache_ctl, CSR_MCACHE_CTL)
DECLARE_CSR(mcctlbeginaddr, CSR_MCCTLBEGINADDR)
DECLARE_CSR(mcctlcommand, CSR_MCCTLCOMMAND)
DECLARE_CSR(mcctldata, CSR_MCCTLDATA)
DECLARE_CSR(sstatus, CSR_SSTATUS)
DECLARE_CSR(sedeleg, CSR_SEDELEG)
DECLARE_CSR(sideleg, CSR_SIDELEG)
DECLARE_CSR(sie, CSR_SIE)
DECLARE_CSR(stvec, CSR_STVEC)
DECLARE_CSR(scounteren, CSR_SCOUNTEREN)
DECLARE_CSR(sscratch, CSR_SSCRATCH)
DECLARE_CSR(sepc, CSR_SEPC)
DECLARE_CSR(scause, CSR_SCAUSE)
DECLARE_CSR(stval, CSR_STVAL)
DECLARE_CSR(sip, CSR_SIP)
DECLARE_CSR(satp, CSR_SATP)
DECLARE_CSR(vsstatus, CSR_VSSTATUS)
DECLARE_CSR(vsie, CSR_VSIE)
DECLARE_CSR(vstvec, CSR_VSTVEC)
DECLARE_CSR(vsscratch, CSR_VSSCRATCH)
DECLARE_CSR(vsepc, CSR_VSEPC)
DECLARE_CSR(vscause, CSR_VSCAUSE)
DECLARE_CSR(vstval, CSR_VSTVAL)
DECLARE_CSR(vsip, CSR_VSIP)
DECLARE_CSR(vsatp, CSR_VSATP)
DECLARE_CSR(hstatus, CSR_HSTATUS)
DECLARE_CSR(hedeleg, CSR_HEDELEG)
DECLARE_CSR(hideleg, CSR_HIDELEG)
DECLARE_CSR(hie, CSR_HIE)
DECLARE_CSR(htimedelta, CSR_HTIMEDELTA)
DECLARE_CSR(hcounteren, CSR_HCOUNTEREN)
DECLARE_CSR(hgeie, CSR_HGEIE)
DECLARE_CSR(htval, CSR_HTVAL)
DECLARE_CSR(hip, CSR_HIP)
DECLARE_CSR(hvip, CSR_HVIP)
DECLARE_CSR(htinst, CSR_HTINST)
DECLARE_CSR(hgatp, CSR_HGATP)
DECLARE_CSR(hgeip, CSR_HGEIP)
DECLARE_CSR(utvt, CSR_UTVT)
DECLARE_CSR(unxti, CSR_UNXTI)
DECLARE_CSR(uintstatus, CSR_UINTSTATUS)
DECLARE_CSR(uscratchcsw, CSR_USCRATCHCSW)
DECLARE_CSR(uscratchcswl, CSR_USCRATCHCSWL)
DECLARE_CSR(stvt, CSR_STVT)
DECLARE_CSR(snxti, CSR_SNXTI)
DECLARE_CSR(sintstatus, CSR_SINTSTATUS)
DECLARE_CSR(sscratchcsw, CSR_SSCRATCHCSW)
DECLARE_CSR(sscratchcswl, CSR_SSCRATCHCSWL)
DECLARE_CSR(mtvt, CSR_MTVT)
DECLARE_CSR(mnxti, CSR_MNXTI)
DECLARE_CSR(mintstatus, CSR_MINTSTATUS)
DECLARE_CSR(mscratchcsw, CSR_MSCRATCHCSW)
DECLARE_CSR(mscratchcswl, CSR_MSCRATCHCSWL)
DECLARE_CSR(mstatus, CSR_MSTATUS)
DECLARE_CSR(misa, CSR_MISA)
DECLARE_CSR(medeleg, CSR_MEDELEG)
DECLARE_CSR(mideleg, CSR_MIDELEG)
DECLARE_CSR(mie, CSR_MIE)
DECLARE_CSR(mtvec, CSR_MTVEC)
DECLARE_CSR(mcounteren, CSR_MCOUNTEREN)
DECLARE_CSR(mcountinhibit, CSR_MCOUNTINHIBIT)
DECLARE_CSR(mscratch, CSR_MSCRATCH)
DECLARE_CSR(mepc, CSR_MEPC)
DECLARE_CSR(mcause, CSR_MCAUSE)
DECLARE_CSR(mtval, CSR_MTVAL)
DECLARE_CSR(mip, CSR_MIP)
DECLARE_CSR(mtinst, CSR_MTINST)
DECLARE_CSR(mtval2, CSR_MTVAL2)
DECLARE_CSR(pmpcfg0, CSR_PMPCFG0)
DECLARE_CSR(pmpcfg1, CSR_PMPCFG1)
DECLARE_CSR(pmpcfg2, CSR_PMPCFG2)
DECLARE_CSR(pmpcfg3, CSR_PMPCFG3)
DECLARE_CSR(pmpaddr0, CSR_PMPADDR0)
DECLARE_CSR(pmpaddr1, CSR_PMPADDR1)
DECLARE_CSR(pmpaddr2, CSR_PMPADDR2)
DECLARE_CSR(pmpaddr3, CSR_PMPADDR3)
DECLARE_CSR(pmpaddr4, CSR_PMPADDR4)
DECLARE_CSR(pmpaddr5, CSR_PMPADDR5)
DECLARE_CSR(pmpaddr6, CSR_PMPADDR6)
DECLARE_CSR(pmpaddr7, CSR_PMPADDR7)
DECLARE_CSR(pmpaddr8, CSR_PMPADDR8)
DECLARE_CSR(pmpaddr9, CSR_PMPADDR9)
DECLARE_CSR(pmpaddr10, CSR_PMPADDR10)
DECLARE_CSR(pmpaddr11, CSR_PMPADDR11)
DECLARE_CSR(pmpaddr12, CSR_PMPADDR12)
DECLARE_CSR(pmpaddr13, CSR_PMPADDR13)
DECLARE_CSR(pmpaddr14, CSR_PMPADDR14)
DECLARE_CSR(pmpaddr15, CSR_PMPADDR15)
DECLARE_CSR(tselect, CSR_TSELECT)
DECLARE_CSR(tdata1, CSR_TDATA1)
DECLARE_CSR(tdata2, CSR_TDATA2)
DECLARE_CSR(tdata3, CSR_TDATA3)
DECLARE_CSR(dcsr, CSR_DCSR)
DECLARE_CSR(dpc, CSR_DPC)
DECLARE_CSR(dscratch0, CSR_DSCRATCH0)
DECLARE_CSR(dscratch1, CSR_DSCRATCH1)
DECLARE_CSR(mhsp_ctl, CSR_MHSP_CTL)
DECLARE_CSR(msp_bound, CSR_MSP_BOUND)
DECLARE_CSR(msp_base, CSR_MSP_BASE)
DECLARE_CSR(mcycle, CSR_MCYCLE)
DECLARE_CSR(minstret, CSR_MINSTRET)
DECLARE_CSR(mhpmcounter3, CSR_MHPMCOUNTER3)
DECLARE_CSR(mhpmcounter4, CSR_MHPMCOUNTER4)
DECLARE_CSR(mhpmcounter5, CSR_MHPMCOUNTER5)
DECLARE_CSR(mhpmcounter6, CSR_MHPMCOUNTER6)
DECLARE_CSR(mhpmcounter7, CSR_MHPMCOUNTER7)
DECLARE_CSR(mhpmcounter8, CSR_MHPMCOUNTER8)
DECLARE_CSR(mhpmcounter9, CSR_MHPMCOUNTER9)
DECLARE_CSR(mhpmcounter10, CSR_MHPMCOUNTER10)
DECLARE_CSR(mhpmcounter11, CSR_MHPMCOUNTER11)
DECLARE_CSR(mhpmcounter12, CSR_MHPMCOUNTER12)
DECLARE_CSR(mhpmcounter13, CSR_MHPMCOUNTER13)
DECLARE_CSR(mhpmcounter14, CSR_MHPMCOUNTER14)
DECLARE_CSR(mhpmcounter15, CSR_MHPMCOUNTER15)
DECLARE_CSR(mhpmcounter16, CSR_MHPMCOUNTER16)
DECLARE_CSR(mhpmcounter17, CSR_MHPMCOUNTER17)
DECLARE_CSR(mhpmcounter18, CSR_MHPMCOUNTER18)
DECLARE_CSR(mhpmcounter19, CSR_MHPMCOUNTER19)
DECLARE_CSR(mhpmcounter20, CSR_MHPMCOUNTER20)
DECLARE_CSR(mhpmcounter21, CSR_MHPMCOUNTER21)
DECLARE_CSR(mhpmcounter22, CSR_MHPMCOUNTER22)
DECLARE_CSR(mhpmcounter23, CSR_MHPMCOUNTER23)
DECLARE_CSR(mhpmcounter24, CSR_MHPMCOUNTER24)
DECLARE_CSR(mhpmcounter25, CSR_MHPMCOUNTER25)
DECLARE_CSR(mhpmcounter26, CSR_MHPMCOUNTER26)
DECLARE_CSR(mhpmcounter27, CSR_MHPMCOUNTER27)
DECLARE_CSR(mhpmcounter28, CSR_MHPMCOUNTER28)
DECLARE_CSR(mhpmcounter29, CSR_MHPMCOUNTER29)
DECLARE_CSR(mhpmcounter30, CSR_MHPMCOUNTER30)
DECLARE_CSR(mhpmcounter31, CSR_MHPMCOUNTER31)
DECLARE_CSR(mhpmevent3, CSR_MHPMEVENT3)
DECLARE_CSR(mhpmevent4, CSR_MHPMEVENT4)
DECLARE_CSR(mhpmevent5, CSR_MHPMEVENT5)
DECLARE_CSR(mhpmevent6, CSR_MHPMEVENT6)
DECLARE_CSR(mhpmevent7, CSR_MHPMEVENT7)
DECLARE_CSR(mhpmevent8, CSR_MHPMEVENT8)
DECLARE_CSR(mhpmevent9, CSR_MHPMEVENT9)
DECLARE_CSR(mhpmevent10, CSR_MHPMEVENT10)
DECLARE_CSR(mhpmevent11, CSR_MHPMEVENT11)
DECLARE_CSR(mhpmevent12, CSR_MHPMEVENT12)
DECLARE_CSR(mhpmevent13, CSR_MHPMEVENT13)
DECLARE_CSR(mhpmevent14, CSR_MHPMEVENT14)
DECLARE_CSR(mhpmevent15, CSR_MHPMEVENT15)
DECLARE_CSR(mhpmevent16, CSR_MHPMEVENT16)
DECLARE_CSR(mhpmevent17, CSR_MHPMEVENT17)
DECLARE_CSR(mhpmevent18, CSR_MHPMEVENT18)
DECLARE_CSR(mhpmevent19, CSR_MHPMEVENT19)
DECLARE_CSR(mhpmevent20, CSR_MHPMEVENT20)
DECLARE_CSR(mhpmevent21, CSR_MHPMEVENT21)
DECLARE_CSR(mhpmevent22, CSR_MHPMEVENT22)
DECLARE_CSR(mhpmevent23, CSR_MHPMEVENT23)
DECLARE_CSR(mhpmevent24, CSR_MHPMEVENT24)
DECLARE_CSR(mhpmevent25, CSR_MHPMEVENT25)
DECLARE_CSR(mhpmevent26, CSR_MHPMEVENT26)
DECLARE_CSR(mhpmevent27, CSR_MHPMEVENT27)
DECLARE_CSR(mhpmevent28, CSR_MHPMEVENT28)
DECLARE_CSR(mhpmevent29, CSR_MHPMEVENT29)
DECLARE_CSR(mhpmevent30, CSR_MHPMEVENT30)
DECLARE_CSR(mhpmevent31, CSR_MHPMEVENT31)
DECLARE_CSR(mvendorid, CSR_MVENDORID)
DECLARE_CSR(marchid, CSR_MARCHID)
DECLARE_CSR(mimpid, CSR_MIMPID)
DECLARE_CSR(mhartid, CSR_MHARTID)
DECLARE_CSR(htimedeltah, CSR_HTIMEDELTAH)
DECLARE_CSR(cycleh, CSR_CYCLEH)
DECLARE_CSR(timeh, CSR_TIMEH)
DECLARE_CSR(instreth, CSR_INSTRETH)
DECLARE_CSR(hpmcounter3h, CSR_HPMCOUNTER3H)
DECLARE_CSR(hpmcounter4h, CSR_HPMCOUNTER4H)
DECLARE_CSR(hpmcounter5h, CSR_HPMCOUNTER5H)
DECLARE_CSR(hpmcounter6h, CSR_HPMCOUNTER6H)
DECLARE_CSR(hpmcounter7h, CSR_HPMCOUNTER7H)
DECLARE_CSR(hpmcounter8h, CSR_HPMCOUNTER8H)
DECLARE_CSR(hpmcounter9h, CSR_HPMCOUNTER9H)
DECLARE_CSR(hpmcounter10h, CSR_HPMCOUNTER10H)
DECLARE_CSR(hpmcounter11h, CSR_HPMCOUNTER11H)
DECLARE_CSR(hpmcounter12h, CSR_HPMCOUNTER12H)
DECLARE_CSR(hpmcounter13h, CSR_HPMCOUNTER13H)
DECLARE_CSR(hpmcounter14h, CSR_HPMCOUNTER14H)
DECLARE_CSR(hpmcounter15h, CSR_HPMCOUNTER15H)
DECLARE_CSR(hpmcounter16h, CSR_HPMCOUNTER16H)
DECLARE_CSR(hpmcounter17h, CSR_HPMCOUNTER17H)
DECLARE_CSR(hpmcounter18h, CSR_HPMCOUNTER18H)
DECLARE_CSR(hpmcounter19h, CSR_HPMCOUNTER19H)
DECLARE_CSR(hpmcounter20h, CSR_HPMCOUNTER20H)
DECLARE_CSR(hpmcounter21h, CSR_HPMCOUNTER21H)
DECLARE_CSR(hpmcounter22h, CSR_HPMCOUNTER22H)
DECLARE_CSR(hpmcounter23h, CSR_HPMCOUNTER23H)
DECLARE_CSR(hpmcounter24h, CSR_HPMCOUNTER24H)
DECLARE_CSR(hpmcounter25h, CSR_HPMCOUNTER25H)
DECLARE_CSR(hpmcounter26h, CSR_HPMCOUNTER26H)
DECLARE_CSR(hpmcounter27h, CSR_HPMCOUNTER27H)
DECLARE_CSR(hpmcounter28h, CSR_HPMCOUNTER28H)
DECLARE_CSR(hpmcounter29h, CSR_HPMCOUNTER29H)
DECLARE_CSR(hpmcounter30h, CSR_HPMCOUNTER30H)
DECLARE_CSR(hpmcounter31h, CSR_HPMCOUNTER31H)
DECLARE_CSR(mstatush, CSR_MSTATUSH)
DECLARE_CSR(mcycleh, CSR_MCYCLEH)
DECLARE_CSR(minstreth, CSR_MINSTRETH)
DECLARE_CSR(mhpmcounter3h, CSR_MHPMCOUNTER3H)
DECLARE_CSR(mhpmcounter4h, CSR_MHPMCOUNTER4H)
DECLARE_CSR(mhpmcounter5h, CSR_MHPMCOUNTER5H)
DECLARE_CSR(mhpmcounter6h, CSR_MHPMCOUNTER6H)
DECLARE_CSR(mhpmcounter7h, CSR_MHPMCOUNTER7H)
DECLARE_CSR(mhpmcounter8h, CSR_MHPMCOUNTER8H)
DECLARE_CSR(mhpmcounter9h, CSR_MHPMCOUNTER9H)
DECLARE_CSR(mhpmcounter10h, CSR_MHPMCOUNTER10H)
DECLARE_CSR(mhpmcounter11h, CSR_MHPMCOUNTER11H)
DECLARE_CSR(mhpmcounter12h, CSR_MHPMCOUNTER12H)
DECLARE_CSR(mhpmcounter13h, CSR_MHPMCOUNTER13H)
DECLARE_CSR(mhpmcounter14h, CSR_MHPMCOUNTER14H)
DECLARE_CSR(mhpmcounter15h, CSR_MHPMCOUNTER15H)
DECLARE_CSR(mhpmcounter16h, CSR_MHPMCOUNTER16H)
DECLARE_CSR(mhpmcounter17h, CSR_MHPMCOUNTER17H)
DECLARE_CSR(mhpmcounter18h, CSR_MHPMCOUNTER18H)
DECLARE_CSR(mhpmcounter19h, CSR_MHPMCOUNTER19H)
DECLARE_CSR(mhpmcounter20h, CSR_MHPMCOUNTER20H)
DECLARE_CSR(mhpmcounter21h, CSR_MHPMCOUNTER21H)
DECLARE_CSR(mhpmcounter22h, CSR_MHPMCOUNTER22H)
DECLARE_CSR(mhpmcounter23h, CSR_MHPMCOUNTER23H)
DECLARE_CSR(mhpmcounter24h, CSR_MHPMCOUNTER24H)
DECLARE_CSR(mhpmcounter25h, CSR_MHPMCOUNTER25H)
DECLARE_CSR(mhpmcounter26h, CSR_MHPMCOUNTER26H)
DECLARE_CSR(mhpmcounter27h, CSR_MHPMCOUNTER27H)
DECLARE_CSR(mhpmcounter28h, CSR_MHPMCOUNTER28H)
DECLARE_CSR(mhpmcounter29h, CSR_MHPMCOUNTER29H)
DECLARE_CSR(mhpmcounter30h, CSR_MHPMCOUNTER30H)
DECLARE_CSR(mhpmcounter31h, CSR_MHPMCOUNTER31H)
#endif
#ifdef DECLARE_CAUSE
DECLARE_CAUSE("misaligned fetch", CAUSE_MISALIGNED_FETCH)
DECLARE_CAUSE("fetch access", CAUSE_FETCH_ACCESS)
DECLARE_CAUSE("illegal instruction", CAUSE_ILLEGAL_INSTRUCTION)
DECLARE_CAUSE("breakpoint", CAUSE_BREAKPOINT)
DECLARE_CAUSE("misaligned load", CAUSE_MISALIGNED_LOAD)
DECLARE_CAUSE("load access", CAUSE_LOAD_ACCESS)
DECLARE_CAUSE("misaligned store", CAUSE_MISALIGNED_STORE)
DECLARE_CAUSE("store access", CAUSE_STORE_ACCESS)
DECLARE_CAUSE("user_ecall", CAUSE_USER_ECALL)
DECLARE_CAUSE("supervisor_ecall", CAUSE_SUPERVISOR_ECALL)
DECLARE_CAUSE("virtual_supervisor_ecall", CAUSE_VIRTUAL_SUPERVISOR_ECALL)
DECLARE_CAUSE("machine_ecall", CAUSE_MACHINE_ECALL)
DECLARE_CAUSE("fetch page fault", CAUSE_FETCH_PAGE_FAULT)
DECLARE_CAUSE("load page fault", CAUSE_LOAD_PAGE_FAULT)
DECLARE_CAUSE("store page fault", CAUSE_STORE_PAGE_FAULT)
DECLARE_CAUSE("fetch guest page fault", CAUSE_FETCH_GUEST_PAGE_FAULT)
DECLARE_CAUSE("load guest page fault", CAUSE_LOAD_GUEST_PAGE_FAULT)
DECLARE_CAUSE("virtual instruction", CAUSE_VIRTUAL_INSTRUCTION)
DECLARE_CAUSE("store guest page fault", CAUSE_STORE_GUEST_PAGE_FAULT)
DECLARE_CAUSE("stack overflow exception", CAUSE_STACK_OVERFLOW_EXCEPTION)
DECLARE_CAUSE("stack underflow exception", CAUSE_STACK_UNDERFLOW_EXCEPTION)
DECLARE_CAUSE("ncp illegal encoding", CAUSE_NCP_ILLEGAL_ENCODING)
DECLARE_CAUSE("ncp vill invalid inst", CAUSE_NCP_VILL_INVALID_INST)
DECLARE_CAUSE("ncp wr vl vtype", CAUSE_NCP_WR_VL_VTYPE)
DECLARE_CAUSE("ncp rw illegal csr", CAUSE_NCP_RW_ILLEGAL_CSR)
DECLARE_CAUSE("ncp rvv misaligned base", CAUSE_NCP_RVV_MISALIGNED_BASE)
DECLARE_CAUSE("ncp rvv misaligned offset", CAUSE_NCP_RVV_MISALIGNED_OFFSET)
DECLARE_CAUSE("ncp rvv access", CAUSE_NCP_RVV_ACCESS)
DECLARE_CAUSE("ncp rvv invalid same rdrs", CAUSE_NCP_RVV_INVALID_SAME_RDRS)
DECLARE_CAUSE("ncp cust misaligned base", CAUSE_NCP_CUST_MISALIGNED_BASE)
DECLARE_CAUSE("ncp cust invalid param", CAUSE_NCP_CUST_INVALID_PARAM)
DECLARE_CAUSE("ncp cust misaligned stride", CAUSE_NCP_CUST_MISALIGNED_STRIDE)
DECLARE_CAUSE("ncp cust access", CAUSE_NCP_CUST_ACCESS)
DECLARE_CAUSE("tcp illegal encoding", CAUSE_TCP_ILLEGAL_ENCODING)
DECLARE_CAUSE("tcp icmov invalid core", CAUSE_TCP_ICMOV_INVALID_CORE)
DECLARE_CAUSE("tcp access start", CAUSE_TCP_ACCESS_START)
DECLARE_CAUSE("tcp access start icmov", CAUSE_TCP_ACCESS_START_ICMOV)
DECLARE_CAUSE("tcp access end l1", CAUSE_TCP_ACCESS_END_L1)
DECLARE_CAUSE("tcp access end llb", CAUSE_TCP_ACCESS_END_LLB)
DECLARE_CAUSE("tcp invalid param", CAUSE_TCP_INVALID_PARAM)
DECLARE_CAUSE("tcp rw illegal csr", CAUSE_TCP_RW_ILLEGAL_CSR)
#endif
