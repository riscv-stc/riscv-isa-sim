/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#include "StreamMessage.h"
#include <string.h>
#include <iostream>

using namespace Transport;

StreamMessage *StreamMessage::gStreamMessage = nullptr;

/*
 * create gStreamMessage singleton and register to stream map
 */
static __attribute__((constructor)) void REGISTER_INSTANCE() {
  if (StreamMessage::gStreamMessage == nullptr) {
    try {
      StreamMessage::gStreamMessage = new StreamMessage;
      StreamMessage::gStreamMessage->registerInstance(
          Interface::STREAM_MESSAGE, StreamMessage::gStreamMessage);
    } catch (std::bad_alloc &e) {
      std::cout << "fail to alloc:" << e.what();
    }
  }
}

/*
 * configure information of queue
 */
bool StreamMessage::setQueueInfo(char *bufferHost, uint32_t bufferRV,
                                 uint32_t bufferSize, uint32_t QHeadRV,
                                 uint32_t QCount, uint32_t maxMsgs) {
  if (bufferSize == 0) {
    std::cout << "rx buffer is empty" << std::endl;
    return false;
  }

  mBufferHost = bufferHost;
  mBufferRV = bufferRV;
  mBufferSize = bufferSize;
  mQueueInfo.headRV = QHeadRV;
  mQueueInfo.count = QCount;
  mMaxMsgs = maxMsgs;

  return true;
}

/*
 * get information of queue
 */
bool StreamMessage::getQueueInfo(uint32_t &headRV, uint32_t &msgCount) {
  headRV = mQueueInfo.headRV;
  msgCount = mQueueInfo.count;

  return true;
}

/*
 * map a risc-v address to host address
 */
void *StreamMessage::toHostAddr(uint32_t rvAddr) {
  return (char *)mBufferHost + (rvAddr - mBufferRV);
}

/*
 * enqueue a data to message queue
 */
bool StreamMessage::enqueue(uint16_t coreId, const char *data,
                            uint32_t dataSize) {
  // validate for message queue
  if (mBufferSize <= sizeof(MessageHeader) * mMaxMsgs) {
    std::cout << "size of message queue is too small:" << mBufferSize
              << std::endl;
    return false;
  }

  // validate for new data
  if (dataSize > (int)(mBufferSize - sizeof(MessageHeader) * mMaxMsgs)) {
    std::cout << "size of new data is too big:" << dataSize << std::endl;
    return false;
  }

  // if no message in message queue, head message is set to start address of
  // message area
  if (mQueueInfo.count == 0) {
    mQueueInfo.headRV = mBufferRV;
  }

  // start address(risc-v) of message queue's buffer(i.e. message header area)
  uint32_t queueRV = mBufferRV;
  // start address(resc-v) of head message in message header area
  uint32_t headRV = mQueueInfo.headRV;
  // start address(risc-v) of message body area
  uint32_t bodiesRV = ALIGN_16(queueRV + sizeof(MessageHeader) * mMaxMsgs);

  int headIdx = 0;  // index of head message
  MessageHeader *headHost =
      toHostAddr(headRV);   // address(host) of head message
  int tailIdx = 0;          // index of tail message
  MessageHeader *tailHost;  // address(host) of tail message
  int newIdx = 0;           // index of new message
  MessageHeader *newHost;   // address(host) of new message

  /*
   *  calculate index of head/tail/new message
   */
  headIdx = (headRV - queueRV) / sizeof(MessageHeader);
  if (mQueueInfo.count == 0) {
    tailIdx = headIdx;
    newIdx = headIdx;
  } else {
    tailIdx = (headIdx + mQueueInfo.count - 1) % mMaxMsgs;
    newIdx = (tailIdx + 1) % mMaxMsgs;
  }

  /*
   *  calculate address(host) of tail/new message's header
   */
  newHost = toHostAddr(queueRV + newIdx * sizeof(MessageHeader));
  tailHost = toHostAddr(queueRV + tailIdx * sizeof(MessageHeader));

  /*
   * calculate address(risc-v) of new message's body
   */
  // by default, address of message's body is set to start address of body area
  newHost->bodyRV = bodiesRV;
  if (mQueueInfo.count != 0) {
    // if free buffer in body area is enough for new message,
    // update address of message's body
    if (queueRV + mBufferSize > tailHost->bodyTailRV + dataSize - 1)
      newHost->bodyRV = tailHost->bodyTailRV;

    // link new message to tail message
    tailHost->nextRV = queueRV + newIdx * sizeof(MessageHeader);
  }
  newHost->bodyTailRV = ALIGN_16(newHost->bodyRV + dataSize);

  /*
   * message header/body area is managed as circle buffer, so new message may
   * overlay oldest message
   */

  // free body buffer is not enough, so body of new message is set to start
  // address of body area. Thus, the head message behand free body buffer should
  // be overlayed firstly.
  // |----|   |----|   |----|
  // |new |...|tail|...|head|...
  // |____|   |____|   |____|
  while (mQueueInfo.count > 0 && newHost->bodyRV <= tailHost->bodyRV &&
         newHost->bodyRV < headHost->bodyRV &&
         headHost->bodyRV > tailHost->bodyRV) {
    // overlay the oldest message's body
    mQueueInfo.headRV = headHost->nextRV;
    headHost = toHostAddr(mQueueInfo.headRV);
    mQueueInfo.count--;
  }

  // overlay from head of message
  // case1:
  // |----|   |----|   |----|
  // |new |...|head|...|tail|...
  // |____|   |____|   |____|
  //
  // case2:
  // |----|   |----|   |----|
  // |tail|...|new |...|head|...
  // |____|   |____|   |____|
  while (mQueueInfo.count > 0 && newHost->bodyRV <= headHost->bodyRV &&
         newHost->bodyTailRV > headHost->bodyRV) {
    // overlay the oldest message's body
    mQueueInfo.headRV = headHost->nextRV;
    headHost = toHostAddr(mQueueInfo.headRV);
    mQueueInfo.count--;
  }

  // set new message's header and body
  newHost->nextRV = 0;
  newHost->size = dataSize;
  newHost->coreId = coreId;
  memcpy((char *)toHostAddr(newHost->bodyRV), data, dataSize);
  mQueueInfo.count++;

#if 1  // debug information
  auto debugHead = mQueueInfo.headRV;
  auto debugCount = mQueueInfo.count;
  while (debugCount != 0) {
    MessageHeader *debugHost = toHostAddr(debugHead);
    std::cout << "body index: " << debugCount - 1
              << " body size: " << debugHost->size
              << " body address: " << debugHost->bodyRV << " body content: "
              << std::string((char *)toHostAddr(debugHost->bodyRV),
                             debugHost->size)
              << std::endl;
    debugHead = debugHost->nextRV;
    debugCount--;
  }
#endif
  return true;
}
