/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#include "Stream.h"
#include "StreamDumpMem.h"
#include "StreamMessage.h"

using namespace Transport;

std::map<Interface::StreamType, Stream*> Stream::gStreamMap;

/**
 * get singleton instance of a stream
 */
Stream* Stream::getInstance(Interface::StreamType streamType) {
  return gStreamMap[streamType];
}

sim_t* Stream::gSim = nullptr;

/**
 * initialize stream
 */
bool Stream::init(sim_t* sim) {
  gSim = sim;
  return true;
}

/**
 * register a stream to stream map
 */
bool Stream::registerInstance(Interface::StreamType streamType,
                              Stream* instance) {
  gStreamMap[streamType] = instance;
  return true;
}

/**
 * post function of recv function in BSP module
 */
bool Stream::recvPost(uint16_t coreId, uint32_t targetAddr, const char* data, uint32_t dataSize,
                      Interface::StreamType streamType, uint16_t tag) {
// spike must only emulate one processor
  if (!gSim || gSim->nprocs() != 1) {
    std::cout << "can't work for spike simulating more than one processor"
              << std::endl;
    return false;
  }
  auto proc = gSim->get_core(0);

  // Message stream
  if (streamType == Interface::STREAM_MESSAGE) {
    // read queue information from CSRs
    uint32_t bufRV = proc->get_csr(CSR_TRBASE);
    uint32_t bufSize = proc->get_csr(CSR_TRSZ);
    uint32_t QHead = proc->get_csr(CSR_TRQHEAD);
    uint32_t QCount = proc->get_csr(CSR_TRQSIZE);
    uint32_t QMax = proc->get_csr(CSR_TRMMC);

    auto streamMessage = dynamic_cast<StreamMessage*>(this);
    if (streamMessage == nullptr) {
      std::cout << "fail to get stream message instance" << std::endl;
      return false;
    }

    // configure queue information
    if (!streamMessage->setQueueInfo(
            dynamic_cast<simif_t*>(gSim)->addr_to_mem(bufRV), bufRV, bufSize,
            QHead, QCount, QMax)) {
      std::cout << "fail to get message queue info" << std::endl;
      return false;
    }

    // enqueue new data
    streamMessage->enqueue(coreId, data, dataSize, tag);

    // update latest queue information to CSRs
    if (streamMessage->getQueueInfo(QHead, QCount)) {
      proc->set_csr(CSR_TRQHEAD, QHead);
      proc->set_csr(CSR_TRQSIZE, QCount);
    }
    return true;
  } else {
    // Rdma stream
    uint32_t base = proc->get_csr(CSR_TRBASE);
    char *dst = dynamic_cast<simif_t*>(gSim)->addr_to_mem(base + targetAddr);
    memcpy(dst, data, dataSize);

    uint32_t qcount = proc->get_csr(CSR_TRQSIZE);
    proc->set_csr(CSR_TRQSIZE, qcount + 1);

    return true;
  }
}

/**
 * @brief dump memory in target to data
 */
bool Stream::dump(uint32_t addr, uint32_t size, std::string* data) {
  auto streamDump = dynamic_cast<StreamDumpMem*>(this);
  streamDump->dump(dynamic_cast<simif_t*>(gSim)->addr_to_mem(addr), size, data);
  return false;
}
