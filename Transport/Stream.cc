/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#include "Stream.h"
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
bool Stream::recvPost(uint16_t coreId, const char* data, uint32_t dataSize,
                      Interface::StreamType streamType) {
  // Message stream
  if (streamType == Interface::STREAM_MESSAGE) {
    // spike must only emulate one processor
    if (!gSim || gSim->nprocs() != 1) {
      std::cout << "can't work for spike simulating more than one processor"
                << std::endl;
      return false;
    }
    auto proc = gSim->get_core(0);

    // read queue information from CSRs
    uint32_t bufRV = proc->get_csr(CSR_TRBASE);
    uint32_t bufSize = proc->get_csr(CSR_TRSZ);
    uint32_t QHead = proc->get_csr(CSR_TRQHEAD);
    uint32_t QCount = proc->get_csr(CSR_TRQSIZE);
    uint32_t QMax = proc->get_csr(CSR_TRMMC);

    auto streamMessage = dynamic_cast<StreamMessage*>(this);
    // configure queue information
    if (!streamMessage->setQueueInfo(
            dynamic_cast<simif_t*>(gSim)->addr_to_mem(bufRV), bufRV, bufSize,
            QHead, QCount, QMax)) {
      std::cout << "fail to get message queue info" << std::endl;
      return false;
    }

    // enqueue new data
    streamMessage->enqueue(coreId, data, dataSize);

    // update latest queue information to CSRs
    if (streamMessage->getQueueInfo(QHead, QCount)) {
      proc->set_csr(CSR_TRQHEAD, QHead);
      proc->set_csr(CSR_TRQSIZE, QCount);
    }
    return true;
  } else {
    // Rdma stream
    return true;
  }
}