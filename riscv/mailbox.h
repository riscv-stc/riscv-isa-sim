#ifndef _STC_MAILBOX_H_
#define _STC_MAILBOX_H_

// #include "common.h"
// #include "type.h"

/******* NPUv2 mailbox *******/

/* a53 has two mailboxs with the same functionality. however, they
 * are designed to cop with messages originating from pcie and npcs
 * respectively, in the sake of responsiveness.
 */

#define P2AP_MBOX_LOC_BASE        0xd3d04000
#define N2AP_MBOX_LOC_BASE        0xd3d06000
#define P2AP_MBOX_IRQ             34
#define N2AP_MBOX_IRQ             35

#define PCIE_MBOX_LOC_PF_BASE		0xE30B0000

/* mailbox registers */
#define MBOX_TX_CFG				(0x0 ) 
#define MBOX_TX_DATA			(0x8 )
#define MBOX_RX_CFG_DATA		(0x10)
#define MBOX_STATUS				(0x18)	/* 32-bit */
#define MBOX_INT_PEND			(0x20)	/* 32-bit */
#define MBOX_INT_MASK			(0x28)	/* 32-bit */

#define MBOX_INT_TX_DONE		(1ul << 0)
#define MBOX_INT_TX_FIFO_OVR	(1ul << 1)
#define MBOX_INT_RX_VALID		(1ul << 2)
#define MBOX_INT_RX_FIFO_UDR	(1ul << 3)
#define MBOX_INT_TX_CFG_ERR		(1ul << 4)
#define MBOX_INT_TX_TO_OVR		(1ul << 5)
#define MBOX_INT_ALL			(0x3f)

#define MBOX_RX_VALID(base)	\
	(readl(base + MBOX_INT_PEND) & MBOX_INT_RX_VALID)

/* 64-bit, little endian layout */
struct mbox_cfg {
	uint64_t src:8;
	uint64_t dst:8;
	uint64_t bcast:1;
	uint64_t type:7;
	uint64_t cust:39;
	uint64_t ver:1;
};

/******* NPUv1 mailbox *******/

#define MBOX_RETRY_DELAY (10)

#define MBOX_MTXCFG_OFFSET 0x0000
#define MBOX_MTXCMD_OFFSET 0x0004
#define MBOX_MTXCMDEXT_OFFSET 0x0008
#define MBOX_MRXCMD_OFFSET 0x000c
#define MBOX_MRXCMDEXT_OFFSET 0x0010
#define MBOX_DTXCFG_OFFSET 0x0014
#define MBOX_DTXREG_OFFSET 0x0018
#define MBOX_STATUS_OFFSET 0x001c
#define MBOX_INT_PEND_OFFSET 0x0020
#define MBOX_INT_MASK_OFFSET 0x0024

// cmd dst
#define CMD_DST_PCIE0 (0xC60A100C)
#define CMD_DST_AP (0xC1F0100C) // a53
#define CMD_DST_NPC_OFFSET (0x7f500c)
#define CMD_DST_NPC(x) (npc_base[x] + CMD_DST_NPC_OFFSET)

// cmd
#define TX_CMD_CSTM_MASK GENMASK(31, 16)
#define TX_CMD_CSTM_SHIFT 16
#define TX_CMD_CODE_MASK GENMASK(15, 8)
#define TX_CMD_CODE_SHIFT 8
#define TX_CMD_SEQ_ID_MASK GENMASK(7, 6)
#define TX_CMD_SEQ_ID_SHIFT 6
#define TX_CMD_SRC_ID_MASK GENMASK(5, 0)
#define TX_CMD_SRC_ID_SHIFT 0

#define MBOX_CMD(cstm, code, seq, src)                                         \
	((((cstm) << TX_CMD_CSTM_SHIFT) & TX_CMD_CSTM_MASK) |                  \
	 (((code) << TX_CMD_CODE_SHIFT) & TX_CMD_CODE_MASK) |                  \
	 (((seq) << TX_CMD_SEQ_ID_SHIFT) & TX_CMD_SEQ_ID_MASK) |               \
	 (((src) << TX_CMD_SRC_ID_SHIFT) & TX_CMD_SRC_ID_MASK))
// for CSTM(16~31)
#define CUSTOM_CMD_TASKID_MASK GENMASK(3, 0)
#define CUSTOM_CMD(taskid) (taskid & CUSTOM_CMD_TASKID_MASK)

#define MBOX_NPC(x) (x)
#define MBOX_SRC_PCIE0 0x20
#define MBOX_SRC_AP 0x21

#define MBOX_STATUS_RXEFIEO_FULL_FLAG (1 << 30)
#define MBOX_STATUS_RXEFIEO_EMPTY_FLAG (1 << 30)
#define MBOX_STATUS_RXEFIEO_LVL_MASK GENMASK(28, 24)
#define MBOX_STATUS_RXEFIEO_LVL_SHIFT 24
#define MBOX_STATUS_RXCFIEO_FULL_FLAG (1 << 22)
#define MBOX_STATUS_RXCFIEO_EMPTY_FLAG (1 << 21)
#define MBOX_STATUS_RXCFIEO_LVL_MASK GENMASK(20, 16)
#define MBOX_STATUS_RXCFIEO_LVL_SHIFT 16

#define MBOX_STATUS_TXFIEO_FULL_FLAG (1 << 14)
#define MBOX_STATUS_TXFIEO_EMPTY_FLAG (1 << 13)
#define MBOX_STATUS_TXFIEO_LVL_MASK GENMASK(12, 8)
#define MBOX_STATUS_TXFIEO_LVL_SHIFT 8
#define MBOX_STATUS_AXI_WFIEO_EMPTY_FLAG 6
#define MBOX_STATUS_AXI_AFIEO_EMPTY_FLAG 5
#define MBOX_STATUS_TX_TRANS_PEND_MASK GENMASK(4, 0)
#define MBOX_STATUS_TX_TRANS_PEND_SHIFT 0

// INT
#define MBOX_RX_EFIFO_UDR_MASK (1 << 5)
#define MBOX_RX_EFIFO_VAL_MASK (1 << 4)
#define MBOX_RX_CFIFO_UDR_MASK (1 << 3)
#define MBOX_RX_CFIFO_VAL_MASK (1 << 2)
#define MBOX_TX_FIFO_OVR_MASK (1 << 1)
#define MBOX_TX_DONE_MASK (1 << 0)

// MBOX CMD MESSAGE
enum {
	// host to npc
	MBOX_WAKEUP_KERNEL = 0x02,
	MBOX_STOP_KERNEL,

	// host to a53
	MBOX_HOST_TO_NPUCTRL = 0x10,

	// a53 to host
	MBOX_NPUCTRL_TO_HOST = 0x20,
	MBOX_NPUCTRL_TEMP_ALERT = 0x21,
	MBOX_NPUCTRL_VM_ALERT = 0x22,

	// npc to host
	MBOX_KERNEL_DONE = 0x30,
	MBOX_PRI_BUFF_FULL = 0x31,
	MBOX_ABNORMAL_END = 0x32,

	// npc to a53
	MBOX_KERNEL_IDLE = 0x40,
	MBOX_KERNEL_EXCEPTION
};

struct mail_box_device {
	void *base;
};

extern struct mail_box_device mbox_dev;
#if 0
void mbox_init(struct mail_box_device *dev, void *base);
#else
void mbox_init();
void npc_mbox_init(int npcid);
#endif
void mbox_exit(struct mail_box_device *dev);
int mbox_send_command(struct mail_box_device *dev, uint32_t cmd_des,
		      uint8_t taskid, uint8_t code, uint8_t cmd_src);
int mbox_recv_command(struct mail_box_device *dev, uint32_t *data);

/* NPUv2 mailbox */
void mbox_pcie_clear_int();
void mbox_npc_clear_int();
void mbox_sendto_pf(uint64_t cust, uint64_t data);
void mbox_sendto_pf_fast(uint64_t data);
void mbox_sendto_vf(uint8_t vf_id, uint64_t cust, uint64_t data);
void mbox_sendto_npc(uint8_t die_id, uint8_t core_id,
						uint64_t cust, uint64_t data);
void mbox_sendto_npc_raw(uint64_t cfg, uint64_t data);
void mbox_recvfrom_pf(uint64_t *cfg, uint64_t *data);
void mbox_recvfrom_vf(uint64_t *cfg, uint64_t *data);
void mbox_recvfrom_npc(uint64_t *cfg, uint64_t *data);
void mbox_recvfrom_pcie(uint64_t *cfg, uint64_t *data);
int mbox_n2ap_rx_fifo_empty();
void mbox_send_msg(uint64_t cfg, uint64_t data);

#endif
