/*
 * replace main.c in cmodel-tests
 */

#include "common.h"

int coreNum = 1024;

struct MessageHeader {
    uint16_t coreId;  // 消息发送端的核 ID
    uint16_t tag;     // 消息 TAG，用来标示不同类型的消息
    uint32_t size;    // 消息体的数据大小（字节数）
    void *body;  // 指向用于存放消息体数据的缓冲区起始指针，16字节对齐
    void *bodyTail;  //
    void *next;      // 队列中的下一个消息
    uint32_t reserved[3];
} __attribute__((packed));

#define RXBUF_SIZE 430//1000//420
#define RXBUF_MAX_MSG 10

char rs1_msg[100] = "message sent from ID ";
static char rxbuf[RXBUF_SIZE];
int icxfer_p()
{
    int coreId = read_csr(mhartid);
    int rd_targetId = ((coreId+1)%coreNum) << 19;
    int msgLen=strlen(rs1_msg);
    if(coreId >=1024){
	printf("stop run, since too many cores:%d\n",coreId);
        return -1;
    }

    write_csr(trbase, rxbuf);
    write_csr(trsz, RXBUF_SIZE);
    write_csr(trqhead, rxbuf);
    write_csr(trqsize, 0);
    write_csr(trmmc, RXBUF_MAX_MSG);
    printf("init syce start\n");
    asm volatile ("sync\n\t");
    printf("init syce done\n");
    
    int tmpId = coreId;

    if(tmpId >= 1000){
	rs1_msg[msgLen++] = '0' + tmpId/1000;
	tmpId %= 1000;
	rs1_msg[msgLen++] = '0' + tmpId/100;
	tmpId %= 100;
	rs1_msg[msgLen++] = '0' + tmpId/10;
	tmpId %= 10;
    } else  if(tmpId >= 100){
	rs1_msg[msgLen++] = '0' + tmpId/100;
	tmpId %= 100;
	rs1_msg[msgLen++] = '0' + tmpId/10;
	tmpId %= 10;
    } else if(tmpId >= 10){
	rs1_msg[msgLen++] = '0' + tmpId/10;
	tmpId %= 10;
    }
    rs1_msg[msgLen++] = '0' + tmpId;
	
    rs1_msg[msgLen] = '\0';
    

while(1) {

    int count = read_csr(trqsize);
    struct MessageHeader * addr = read_csr(trqhead);
    if(count !=1)
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++error count :%d\n", count);
    while(count--){
	printf("%d receive %d --> %d\n",coreId, addr->coreId, coreId);
	addr = addr->next;
    }

{
    int sleep = 10000000;
    while(sleep-->0) ;
}
    write_csr(trqhead, rxbuf);
    write_csr(trqsize, 0);

    printf("%d send %d --> %d\n", coreId, coreId, rd_targetId>>19);
    icxfer_p_asm(rd_targetId, rs1_msg, strlen(rs1_msg), 1);
    printf("%d sync start\n", coreId);
    asm volatile ("sync\n\t");
    printf("%d sync done\n", coreId);

    //break;
}
    return TEST_SUCCESS;
}

int main()
{
    printf("bsp test Begins..\n");

    icxfer_p();

    // exit simulation
    exit(0);

    return 0;
}
