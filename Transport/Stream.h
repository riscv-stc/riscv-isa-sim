/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#ifndef TRANSPORT_STREAM_H
#define TRANSPORT_STREAM_H

#include "Interface.h"
#include "sim.h"

namespace Transport {

/**
 * @brief stream used in framework
 */
class Stream {
 public:
  virtual ~Stream(){};

  /**
   * @brief get singleton instance of a stream
   * @param streamType: type of stream
   * @return pointer to a stream
   */
  static Stream *getInstance(Interface::StreamType streamType);

  /**
   * @brief stream map
   * key: type of stream
   * value: pointer to a stream
   */
  static std::map<Interface::StreamType, Stream *> gStreamMap;

  /**
   * @brief initialize stream
   * @param sim: pointer to sim_t
   * @return true - success; false - fail
   */
  static bool init(sim_t *sim);
  static sim_t *gSim;  // global pointer to simt_t object

  /**
   * @brief register a stream to stream map
   * @param streamType: type of stream
   * @param instance: pointer to a stream
   * @return true - success; false - fail
   */
  bool registerInstance(Interface::StreamType streamType, Stream *instance);

  /**
   * @brief post function of recv function in BSP module
   * @param coreId: ID of source core(spike)
   * @param targetAddr: target address for rdma
   * @param data: address of data
   * @param dataSize: size of data
   * @param streamType: type of a stream
   * @param tag: message tag
   * @return true - success; false - fail
   */
  bool recvPost(uint16_t coreId, uint32_t targetAddr, const char *data, uint32_t dataSize,
                Interface::StreamType streamType, Interface::StreamDir streamDir, uint16_t tag);

  /**
   * @brief dump memory in target to data
   * @param addr: start address of memory to dump
   * @param size: size of memory to dump
   * @param[out] data: store dumped memory to data
   * @return true - success; false - fail
   */
  bool dump(uint32_t addr, uint32_t size, std::string *data);
};
}

#endif  // TRANSPORT_STREAM_H
