/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 */

#ifndef TRANSPORT_STREAM_MESSAGE_H
#define TRANSPORT_STREAM_MESSAGE_H

#include <string>
#include "Stream.h"

namespace Transport {
/**
 * @brief Message stream
 */
class StreamMessage : public Stream {
 public:
  // basic information of message queue
  struct MessageQueue {
    uint32_t headRV;  // 指向消息队列中第一个消息头的起始地址
    uint32_t count;   // 消息队列中的消息个数
  };

  // information of message header
  struct MessageHeader {
    uint16_t coreId;  // 消息发送端的核ID
    uint16_t tag;     // 消息TAG，用来标示不同类型的消息
    uint32_t size;    // 消息体的数据大小（字节数）
    uint32_t bodyRV;  // 指向用于存放消息体数据的缓冲区起始指针，16字节对齐
    uint32_t bodyTailRV;  // 指向用于下一个存放消息体数据的缓冲区起始指针
    uint32_t nextRV;  // 队列中的下一个消息
    uint32_t reserved[3];
  } __attribute__((packed));

// set address to 16 bytes aligned
#define ALIGN_16(addr) (((addr) + ((16) - 1)) & ~((16) - 1))

  static StreamMessage *gStreamMessage;  // singleton instance of this class

  /**
   * @brief get information of message queue
   * @param[out] headRV: risc-v address of head message in queue
   * @param[out] msgCount: count of message in queue
   * @return true - success; false - fail
   */
  bool getQueueInfo(uint32_t &headRV, uint32_t &msgCount);

  /**
   * @brief configure information of message queue
   * @param bufferHost: host address of queue buffer
   * @param bufferRV: risc-v address of queue buffer
   * @param bufferSize: size of queue buffer
   * @param QHeadRV: risc-v address of head message in queue
   * @param QCount: count of message in queue
   * @param maxMsgs: max count of message in queue
   * @return true - success; false - fail
   */
  bool setQueueInfo(char *bufferHost, uint32_t bufferRV, uint32_t bufferSize,
                    uint32_t QHeadRV, uint32_t QCount, uint32_t maxMsgs);

  /**
   * @brief enqueue a data to message queue
   * @param coreId: ID of core(spike)
   * @param data: address of data
   * @param dataSize: size of data
   * @return true - success; false - fail
   */
  bool enqueue(uint16_t coreId, const char *data, uint32_t dataSize,
               uint16_t tag);

  /**
   * @brief garbage collection to clean gStreamMessage
   */
  class Gc {
   public:
    ~Gc() {
      if (gStreamMessage != nullptr) {
        delete gStreamMessage;
      }
    }
  };

 private:
  /**
   * @brief map a risc-v address to host address
   * @param rvAddr: risc-v address
   * @return host address
   */
  void *toHostAddr(uint32_t rvAddr);

  char *mBufferHost = nullptr;  // address(host) of message queue's buffer
  uint32_t mBufferRV = 0;       // address(risc-v) of message queue's buffer
  uint32_t mBufferSize = 0;     // size of message queue's buffer
  MessageQueue mQueueInfo = {0, 0};  // basic information of message queue
  uint32_t mMaxMsgs = 0;             // max count of message in message queue
};
}

#endif  // TRANSPORT_STREAM_MESSAGE_H
