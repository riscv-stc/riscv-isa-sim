#include "noc_addr.h"

/* NPC local space in soc view. */
static const uint32_t noc_npc_base[] = {
        NOC_NPC0_BASE,
        NOC_NPC1_BASE,
        NOC_NPC2_BASE,
        NOC_NPC3_BASE,
        NOC_NPC4_BASE,
        NOC_NPC5_BASE,
        NOC_NPC6_BASE,
        NOC_NPC7_BASE,
        NOC_NPC8_BASE,
        NOC_NPC9_BASE,
        NOC_NPC10_BASE,
        NOC_NPC11_BASE,
        NOC_NPC12_BASE,
        NOC_NPC13_BASE,
        NOC_NPC14_BASE,
        NOC_NPC15_BASE,
	      NOC_NPC16_BASE,
	      NOC_NPC17_BASE,
	      NOC_NPC18_BASE,
	      NOC_NPC19_BASE,
	      NOC_NPC20_BASE,
	      NOC_NPC21_BASE,
	      NOC_NPC22_BASE,
	      NOC_NPC23_BASE,
	      NOC_NPC24_BASE,
	      NOC_NPC25_BASE,
	      NOC_NPC26_BASE,
	      NOC_NPC27_BASE,
	      NOC_NPC28_BASE,
	      NOC_NPC29_BASE,
	      NOC_NPC30_BASE,
	      NOC_NPC31_BASE
};

#define NOC_NPC_TOTAL \
	(sizeof(noc_npc_base) / \
	sizeof(noc_npc_base[0]))

/* current address is which npc */
#define IS_NPC(addr, id) \
	(((addr) >= noc_npc_base[id]) && \
	((addr) < noc_npc_base[id] + NPC_LOCAL_REGIN_SIZE))

/* change soc address to local address */
#define soc_to_local(addr, id) \
	((addr) - noc_npc_base[id] + NPC_LOCAL_ADDR_START)

/* adjust which npc from addr. */
int which_npc(reg_t addr, reg_t *paddr)
{
  int core_id = -1;
  int mbox_num;

  mbox_num = NOC_NPC_TOTAL;
  for (int i = 0; i < mbox_num; i++) {
    if (IS_NPC(addr, i)) {
      core_id = i;
      *paddr = soc_to_local(addr, i);
      break;
    }
  }

  return core_id;
}