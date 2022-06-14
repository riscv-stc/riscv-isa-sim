#include "ap_mbox.h"

#define RX_CFIFO_VAL     0x2
#define RX_EXT_CFIFO_VAL 0x4

#define PCIE0_MBOX_MRXCMD       (0xC60A100C)
#define PCIE0_MBOX_MRXCMDEXT    (0xC60A1010)

ap_mbox_device_t::ap_mbox_device_t(simif_t* sim, sys_irq_t *sys_irq, pcie_driver_t *pcie, bool pcie_enabled)
    : sim(sim), sys_irq(sys_irq), pcie_driver(pcie), pcie_enabled(pcie_enabled)
{
    cmd_count = 0;
    cmdext_count = 0;
    reset();
}

ap_mbox_device_t::~ap_mbox_device_t()
{

}

void ap_mbox_device_t::reset()
{
    cmd_count = 0;
    cmdext_count = 0;
    memset(reg_base, 0, AP_MBOX_SIZE);

    *(uint32_t *)(reg_base+MBOX_STATUS_OFFSET) = 0x20202060;

    while (!cmd_value.empty())
        cmd_value.pop();

    while (!cmdext_value.empty())
        cmdext_value.pop();
}

bool ap_mbox_device_t::load(reg_t addr, size_t len, uint8_t* bytes)
{
    if (unlikely(!bytes || addr >= size()))
        return false;
    
    switch(addr) {
    case MBOX_INT_PEND:
        *(uint32_t *)bytes = (1<<3);
        break;
    default:
        *(uint32_t *)bytes = 0;
        break;
    }
    #if 0
    switch(addr) {
    case MBOX_MRXCMD_OFFSET:
        {
        if (cmd_value.empty())
            return false;

        uint32_t value = cmd_value.front();
        cmd_value.pop();
        cmd_count--;
        memcpy(bytes, &value, 4);
        }
        break;
    case MBOX_MRXCMDEXT_OFFSET:
        {
        if (cmdext_value.empty())
            return false;

        uint32_t value = cmdext_value.front();
        cmdext_value.pop();
        cmdext_count--;
        memcpy(bytes, &value, 4);
        }
        break;
    default:
        memcpy(bytes, reg_base + addr, len);
    }
    #endif
    return true;
}

bool ap_mbox_device_t::store(reg_t addr, size_t len, const uint8_t* bytes)
{
    if (unlikely(!bytes || addr >= size()))
        return false;
#if 0
    switch(addr) {
    case MBOX_MTXCFG_OFFSET:
        memcpy(reg_base + addr, bytes, len);
        break;
    case MBOX_STATUS_OFFSET:        /* RO */
        return false;
        break;
    case MBOX_INT_PEND_OFFSET:      /* R/W1TC */
    {
        uint32_t v = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
        *(uint32_t *)(reg_base+MBOX_INT_PEND_OFFSET) &= ~(v);
        if ((0!=v) && (0==*(uint32_t *)(reg_base+MBOX_INT_PEND_OFFSET))) {
            sys_irq->generate_irq_to_a53(NPUV2_MBOX_IRQ_ID, 0);
        }
        
        if (!cmd_value.empty()) {
            *(uint32_t *)(reg_base+MBOX_INT_PEND_OFFSET) |= (1 << RX_CFIFO_VAL);
            sys_irq->generate_irq_to_a53(NPUV2_MBOX_IRQ_ID, 0);
        }
        if (!cmdext_value.empty()) {
            *(uint32_t *)(reg_base+MBOX_INT_PEND_OFFSET) |= (1 << RX_EXT_CFIFO_VAL);
            sys_irq->generate_irq_to_a53(NPUV2_MBOX_IRQ_ID, 0);
        }
    }
        break;
    case MBOX_MTXCMD_OFFSET:
    case MBOX_MTXCMDEXT_OFFSET:
        memcpy(reg_base + addr, bytes, len);
        switch(*(uint32_t *)reg_base & 0xfffff) {
        /**
         * PCIE0_MBOX_MRXCMD (0xC60A_100C)
         * AP_MBOX_MRXCMD (0xC1F0_100C)
         * NPCx_MBOX_MRXCMD (NPCx_BASE + 0x007F_500C)
         */
        case PCIE0_MBOX_MRXCMD & 0xfffff:
        case PCIE0_MBOX_MRXCMDEXT & 0xfffff:
            {
            command_head_t cmd;
            cmd.code = CODE_INTERRUPT;
            cmd.addr = addr;
            cmd.len = 4;
            *(uint32_t *)cmd.data = *(uint32_t *)bytes;
            if(pcie_enabled) {
                pcie_driver->send((const uint8_t *)&cmd, sizeof(cmd));
                printf("ap_mbox send to pcie_mbox 0x%x \r\n", *(uint32_t *)bytes);
            }
            }
            break;
        case CMD_DST_NPC_OFFSET & 0xfffff:
            {
            int dst_proc = 0;
            reg_t value = *(uint32_t *)reg_base;
            reg_t mtxcfg = 0;
            dst_proc = which_npc(value, &mtxcfg);
            if (0 <= dst_proc) {
                mtxcfg = (mtxcfg&(AP_MBOX_SIZE-1)) + MBOX_START;
                #if 0
                if (!sim->local_mmio_store(mtxcfg, 4, bytes, dst_proc)) {
                    std::cout << "PCIe driver store addr: 0x"
                        << hex
                        << addr
                        << " access fault."
                        << std::endl;
                    throw trap_store_access_fault(addr);
                }
                #endif
                printf("ap_mbox send to np_mbox 0x%x \r\n", *(uint32_t *)bytes);
            } else {
                printf("unsupport!!!  %s() +%d error txcfg 0x%x \r\n",__FUNCTION__,__LINE__, addr);
            }
            }
            break;
        default:
            printf("unsupport!!!  %s() +%d error txcfg 0x%x \r\n",__FUNCTION__,__LINE__, addr);
            break;
        }
        break;
    case MBOX_MRXCMD_OFFSET:
        memcpy(reg_base + addr, bytes, len);
        cmd_count++;
        cmd_value.push(*(uint32_t*)bytes);
        *(uint32_t *)(reg_base + MBOX_INT_PEND_OFFSET) |= 1 << RX_CFIFO_VAL;
        sys_irq->generate_irq_to_a53(NPUV2_MBOX_IRQ_ID, 0);
        break;
    case MBOX_MRXCMDEXT_OFFSET:
        memcpy(reg_base + addr, bytes, len);
        cmdext_count++;
        cmdext_value.push(*(uint32_t*)bytes);
        *(uint32_t *)(reg_base + MBOX_INT_PEND_OFFSET) |= 1 << RX_EXT_CFIFO_VAL;
        sys_irq->generate_irq_to_a53(NPUV2_MBOX_IRQ_ID, 0);
        break;
    default:
        break;
    }
#endif
    return true;
}
