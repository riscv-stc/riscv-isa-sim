#ifndef __APIFC_H__
#define __APIFC_H__

#include <thread>
#include "simif.h"
#include "pcie_driver.h"

/* spike和qemu通过消息队列通信 */
#define SPIKE_QEMU_MSG_PATHNAME     "/proc/stc/stc_cluster_0"
#define SPIKE_QEMU_MSG_S2Q_PROJ     's'
#define SPIKE_QEMU_MSG_Q2S_PROJ     'q'

/* spike to qemu msg type */
#define SQ_MTYPE_REQ_BASE       0x1000
#define SQ_MTYPE_RES_BASE       0x2000
/* cmd: enum command_code */
#define SQ_MTYPE_REQ(cmd)       (SQ_MTYPE_REQ_BASE + cmd)
#define SQ_MTYPE_RES(cmd)       (SQ_MTYPE_RES_BASE + cmd)

struct sq_msg_t {
    long mtype;
    struct command_head_t cmd_data;
};

#define cmd_data_irq(irq)           (irq)
#define cmd_data_clear_irq(irq)     (irq | (1<<31))         /* 最高位1代表清除中断 */
#define cmd_data_get_irq(data)      (data & (~(1<<31)))
#define is_clear_irq(data)          (data & (1<<31))

class apifc_t
{
public:
    apifc_t(simif_t *sim);
    ~apifc_t();
  
    int sqmsg_spike_send(long recv_type, const struct command_head_t *cmd_data);
    int sqmsg_spike_recv(long recv_type, struct command_head_t *cmd_data);

private:
    simif_t *sim = nullptr;

    int sq_s2q_msqid = -1;        /* spike send to qemu */
    int sq_q2s_msqid = -1;        /* qemu send to spike */
    std::unique_ptr<std::thread> sqmsg_spike_recv_thread_p;
    int qemu_mems_read(reg_t addr, size_t length);
    void sqmsg_req_recv_handle(void);
    int spike_qemu_msg_init(void);
    int spike_qemu_msg_destory(void);

    bool load_data(reg_t addr, size_t len, uint8_t* bytes);
    bool store_data(reg_t addr, size_t len, const uint8_t* bytes);
};

#endif