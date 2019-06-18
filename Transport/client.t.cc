/**
  should modify init.S
init.S
@@ -131,10 +131,10 @@ _start:
   and tp, tp, -64

   # get core id
-  csrr a0, mhartid
+#  csrr a0, mhartid
   # for now, assume only 1 core
   li a1, NUM_CPUS
-1:bgeu a0, a1, 1b
+#1:bgeu a0, a1, 1b

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

#define RXBUF_SIZE 430  // 1000//420
#define RXBUF_MAX_MSG 10

char rs1_msg[100] = "message sent from ID ";
static char rxbuf[RXBUF_SIZE];
int coreId;

int init() {
  coreId = read_csr(mhartid);
  int msgLen = strlen(rs1_msg);
  if (coreId >= 1024) {
    printf("stop run, since too many cores:%d\n", coreId);
    return -1;
  }

  write_csr(trbase, rxbuf);
  write_csr(trsz, RXBUF_SIZE);
  write_csr(trqhead, rxbuf);
  write_csr(trqsize, 0);
  write_csr(trmmc, RXBUF_MAX_MSG);
  printf("init sync start\n");
  asm volatile("sync\n\t");
  printf("init sync done\n");

  int tmpId = coreId;

  if (tmpId >= 1000) {
    rs1_msg[msgLen++] = '0' + tmpId / 1000;
    tmpId %= 1000;
    rs1_msg[msgLen++] = '0' + tmpId / 100;
    tmpId %= 100;
    rs1_msg[msgLen++] = '0' + tmpId / 10;
    tmpId %= 10;
  } else if (tmpId >= 100) {
    rs1_msg[msgLen++] = '0' + tmpId / 100;
    tmpId %= 100;
    rs1_msg[msgLen++] = '0' + tmpId / 10;
    tmpId %= 10;
  } else if (tmpId >= 10) {
    rs1_msg[msgLen++] = '0' + tmpId / 10;
    tmpId %= 10;
  }
  rs1_msg[msgLen++] = '0' + tmpId;

  rs1_msg[msgLen] = '\0';
}

enum bspType {
  S0_TO_S1,
  S0_TO_S256,
  S0_S0_TO_S1,
};

void bsp(int testNUm, enum bspType type) {
  int rd_targetId;
  switch (type) {
    case S0_TO_S1:
      rd_targetId = ((coreId + 1) % coreNum) << 19;
      break;
    case S0_TO_S256:
      rd_targetId = ((coreId + 256) % coreNum) << 19;
      break;
    case S0_S0_TO_S1:
      rd_targetId = ((((coreId + 2) / 2) * 2) % coreNum) << 19;
      break;
  }

  int  bFirst = 1;
  while (testNUm--) {
    int count = read_csr(trqsize);
    struct MessageHeader *addr = read_csr(trqhead);
    switch (type) {
      case S0_TO_S1:
      case S0_TO_S256:
        if (count != 1 && !bFirst) printf("+++++error count should be 1: %d\n", count);
        break;
      case S0_S0_TO_S1:
        if (count != 2  && coreId%2 == 0 && !bFirst) printf("+++++error count should be 2: %d\n", count);
        break;
    }

    while (count--) {
      printf("%d receive %d --> %d\n", coreId, addr->coreId, coreId);
      addr = addr->next;
    }

    write_csr(trqhead, rxbuf);
    write_csr(trqsize, 0);

    mdelay(1);

    if (read_csr(trqsize) > 0)
      printf("+++++error count should be zero:%d\n", count);

    printf("%d send %d --> %d\n", coreId, coreId, rd_targetId >> 19);
    icxfer_p_asm(rd_targetId, rs1_msg, strlen(rs1_msg), 1);
    printf("%d sync start\n", coreId);
    asm volatile("sync\n\t");
    printf("%d sync done\n", coreId);

    bFirst = 0;
  }
}

int main() {
  printf("bsp test Begins..\n");

  int loop = 5;
  init();
  printf("s0 - > s1 test ..\n");
  bsp(loop, S0_TO_S1);
  printf("s0,s1 -> s2 test..\n");
  bsp(loop, S0_S0_TO_S1);
  printf("s0 - > s256 test ..\n");
  bsp(loop, S0_TO_S256);

  // exit simulation
  exit(0);

  return 0;
}

