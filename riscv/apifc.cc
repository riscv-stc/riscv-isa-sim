#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "apifc.h"
#include "processor.h"

apifc_t::apifc_t(simif_t *sim) : sim(sim)
{
    int ret = 0;
    /* 初始化消息队列，创建线程处理qemu发送的请求 */
    spike_qemu_msg_init();
    if (0 != ret) {
        std::cout << "apifc_t() spike_qemu_msg_init failed ret " << ret << std::endl;
        return ;
    }
}

apifc_t::~apifc_t()
{
    spike_qemu_msg_destory();
}

bool apifc_t::load_data(reg_t addr, size_t len, uint8_t* bytes)
{
    reg_t paddr = 0;
    int procid = 0;
    int idxinbank  = 0;
    char *host_addr = nullptr;

    /* npc 8MB buffer */
    procid = which_npc(addr, &paddr);
    if (0 <= procid) {
        int idxinsim =sim->coreid_to_idxinsim(procid);
        int bankid = sim->get_bankid(idxinsim);
        idxinbank = sim->get_idxinbank(idxinsim);

        if ((host_addr=sim->npc_addr_to_mem(paddr, bankid, idxinbank)) || 
                (host_addr=sim->bank_addr_to_mem(paddr, bankid))) {
            memcpy(bytes, host_addr, len);
            return true;
        } else if (!((sim->npc_mmio_load(paddr, len, bytes, bankid, idxinbank)) || 
                (sim->bank_mmio_load(addr, len, bytes, bankid)))) {
            std::cout << "apifc driver load addr: 0x"
                << hex
                << addr
                << " access fault."
                << std::endl;
            throw trap_load_access_fault(false, addr, 0, 0);
            return false;
        }
    }

    if (nullptr != (host_addr=sim->addr_to_mem(addr))) {
        memcpy(bytes, host_addr, len);
    } else if (!sim->mmio_load(addr, len, bytes)) {
        std::cout << "apifc driver load addr: 0x"
            << hex
            << addr
            << " access fault."
            << std::endl;
        throw trap_load_access_fault(false, addr, 0, 0);
        return false;
    }

    return true;
}

bool apifc_t::store_data(reg_t addr, size_t len, const uint8_t* bytes)
{
    reg_t paddr = 0;
    int procid = 0;
    int idxinbank  = 0;
    char *host_addr = nullptr;

    /* npc 8MB buffer */
    procid = which_npc(addr, &paddr);
    if (0 <= procid) {
        int idxinsim =sim->coreid_to_idxinsim(procid);
        int bankid = sim->get_bankid(idxinsim);
        idxinbank = sim->get_idxinbank(idxinsim);

        if ((host_addr=sim->npc_addr_to_mem(paddr, bankid, idxinbank)) || 
                (host_addr=sim->bank_addr_to_mem(paddr, bankid))) {
            memcpy(host_addr, bytes, len);
            return true;
        } else if (!((sim->npc_mmio_store(paddr, len, bytes, bankid, idxinbank)) || 
                (sim->bank_mmio_store(addr, len, bytes, bankid)))) {
            std::cout << "apifc driver store addr: 0x"
                << hex
                << addr
                << " access fault."
                << std::endl;
            throw trap_store_access_fault(false, addr, 0, 0);
            return false;
        }
    }

    if (nullptr != (host_addr=sim->addr_to_mem(addr))) {
        memcpy(host_addr, bytes, len);
    } else if (!sim->mmio_store(addr, len, bytes)) {
        std::cout << "apifc driver store addr: 0x"
            << hex
            << addr
            << " access fault."
            << std::endl;
        throw trap_store_access_fault(false, addr, 0, 0);
        return false;
    }
    
    return true;
}

/* 向qemu发送指定类型的消息，失败返回负数 */
int apifc_t::sqmsg_spike_send(long recv_type, const struct command_head_t *cmd_data)
{
    int send_len = 0;
    struct sq_msg_t sq_msg = {};

    if (NULL == cmd_data) {
        return -1;
    }

    sq_msg.mtype = recv_type;
    send_len = COMMAND_HEAD_SIZE + cmd_data->len;
    if (send_len > (int)sizeof(sq_msg.cmd_data)) {
        send_len = (int)sizeof(sq_msg.cmd_data);
    }
    memcpy(&(sq_msg.cmd_data), cmd_data, send_len);

    return msgsnd(sq_s2q_msqid, &sq_msg, send_len, 0);
}

/* 从qemu接收指定类型的消息，失败返回负数 */
int apifc_t::sqmsg_spike_recv(long recv_type, struct command_head_t *cmd_data)
{
    int ret = 0;
    int recv_len = 0;
    struct sq_msg_t sq_msg = {};

    if (NULL == cmd_data) {
        return -1;
    }

    memset(cmd_data, 0, sizeof(*cmd_data));
    ret = msgrcv(sq_q2s_msqid, &sq_msg, sizeof(sq_msg), recv_type, 0);
    if (0 > ret) {
        perror("sqmsg_req_recv_handle msgrcv() \r\n");
        ret = -2;
    } else {
        recv_len = COMMAND_HEAD_SIZE + sq_msg.cmd_data.len;
        if (recv_len > (int)sizeof(*cmd_data)) {
            recv_len = (int)sizeof(*cmd_data);
        }
        memcpy(cmd_data, &(sq_msg.cmd_data), recv_len);
    }
    return ret;
}

/* 访问bus读取内存，并向qemu发送应答消息 */
int apifc_t::qemu_mems_read(reg_t addr, size_t length)
{
    int count = 0;
    int size = 0;
    int block_size = 4;
    struct command_head_t cmd_data = {};

    for (size_t offset = 0; offset < length; offset += block_size) {
        size = std::min(length - offset, (size_t)block_size);

        memset(&cmd_data, 0, sizeof(cmd_data));
        cmd_data.code = CODE_READ;
        cmd_data.addr = addr + offset;
        cmd_data.len = size;
        load_data(cmd_data.addr, cmd_data.len, (uint8_t*)cmd_data.data);

        sqmsg_spike_send(SQ_MTYPE_RES(CODE_READ), &cmd_data);

        count += size;
    }

    return count;
}

void apifc_t::sqmsg_req_recv_handle(void)
{
    int ret = 0;
    struct command_head_t cmd_data = {};
    long recv_type = 0;
    
    while(1) {
        memset(&cmd_data, 0, sizeof(cmd_data));
        // recv_type = SQ_MTYPE_Q2S_REQ(CODE_READ);
        recv_type = 0;
        ret = sqmsg_spike_recv(recv_type, &cmd_data);
        if (0 > ret) {
            perror("sqmsg_req_recv_handle msgrcv() ");
            sleep(1);
            continue;
        }
        switch(cmd_data.code) {
        case CODE_READ:
            ret = qemu_mems_read(cmd_data.addr, cmd_data.len);
            if (0 > ret) {
                printf("%s() qemu_mems_read ret %d error \r\n",__FUNCTION__,ret);
            }
            printf("ap r 0x%lx l %d \r\n", cmd_data.addr, cmd_data.len);
            break;
        case CODE_WRITE:
            store_data(cmd_data.addr, cmd_data.len, (const uint8_t*)cmd_data.data);
            printf("ap w 0x%lx l %d \r\n", cmd_data.addr, cmd_data.len);
            break;
        default:
            printf("%s() unknow cmd 0x%x \r\n", __FUNCTION__, cmd_data.code);
            break;
        }
    }
}

void apifc_t::spike_qemu_msg_destory(void)
{
    msgctl(sq_s2q_msqid, IPC_RMID, NULL);
    msgctl(sq_q2s_msqid, IPC_RMID, NULL);
    sq_s2q_msqid = -1;
    sq_q2s_msqid = -1;
}

/* 初始化消息队列，并创建线程接收请求，成功返回0 */
int apifc_t::spike_qemu_msg_init(void)
{
    int ret = 0;
    key_t key = 0;
    struct sq_msg_t sq_msg = {};

    /* sq_s2q_msqid */
    key = ftok(SPIKE_QEMU_MSG_PATHNAME, SPIKE_QEMU_MSG_S2Q_PROJ);
    if (-1 == key) {
        printf("%s() get key failed, pathname:%s proj:%c \r\n",
            __FUNCTION__, SPIKE_QEMU_MSG_PATHNAME, SPIKE_QEMU_MSG_S2Q_PROJ);
        return -1;
    }
    sq_s2q_msqid = msgget(key, IPC_CREAT | 0666);
    if (0 > sq_s2q_msqid) {
        sq_s2q_msqid = -1;
        printf("%s() msgget failed ret %d \r\n", __FUNCTION__, sq_s2q_msqid);
        return -2;
    }
    do {
        ret = msgrcv(sq_s2q_msqid, &sq_msg, sizeof(sq_msg), 0, IPC_NOWAIT);
    } while(0 <= ret);

    /* sq_q2s_msqid */
    key = ftok(SPIKE_QEMU_MSG_PATHNAME, SPIKE_QEMU_MSG_Q2S_PROJ);
    if (-1 == key) {
        printf("%s() get key failed, pathname:%s proj:%c \r\n",
            __FUNCTION__, SPIKE_QEMU_MSG_PATHNAME, SPIKE_QEMU_MSG_Q2S_PROJ);
        return -1;
    }
    sq_q2s_msqid = msgget(key, IPC_CREAT | 0666);
    if (0 > sq_q2s_msqid) {
        sq_q2s_msqid = -1;
        printf("%s() msgget failed ret %d \r\n", __FUNCTION__, sq_q2s_msqid);
        return -2;
    }
    do {
        ret = msgrcv(sq_q2s_msqid, &sq_msg, sizeof(sq_msg), 0, IPC_NOWAIT);
    } while(0 <= ret);

    auto sq_req_task_loop = std::bind(&apifc_t::sqmsg_req_recv_handle, this);
    auto sq_req_thread = new std::thread(sq_req_task_loop);
    sq_req_thread->detach();
    sqmsg_spike_recv_thread_p.reset(sq_req_thread);

    return 0;
}