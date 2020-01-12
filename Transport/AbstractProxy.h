/**
 * See LICENSE for license details
 *
 * @author Pascal Ouyang
 *
 */

#ifndef TRANSPORT_ABSTRACT_PROXY_H
#define TRANSPORT_ABSTRACT_PROXY_H

#include <string>

/**
 * Transport namespace packages entire Transport module
 */
namespace Transport {

/**
 * @brief abstract class of Proxy
 */
class AbstractProxy {
 public:

  /**
   * @brief interface class for proxy callback
   */
  class Callback {
  public:
    // Receive data from Transport module
    virtual bool recv(uint32_t dstaddr, const char* data, uint32_t size, bool set_active) = 0;
    virtual bool dump(std::string *data, uint64_t addr, uint32_t size) = 0;
  };

  // direction of stream used in GRPC framework
  enum StreamDir {
    CORE2CORE  = 0,  // from a core(spike) to another core(spike)
    CORE2LLB   = 1,  // from a core(spike) to LLB
    LLB2CORE   = 2,  // from LLB to a core(spike)
  };

  // direction of DMA
  enum DmaDir {
    LLB2DDR  = 0,  // from LLB to DDR
    DDR2LLB  = 1,  // from DDR to LLB
    DDR2DDR  = 2,  // from DDR to DDR
  };

  // pure virtual deconstructor, used for abstract class
  virtual ~AbstractProxy() = 0;

  /**
   * @brief initialize transport
   * @param coreId: ID of core(spike)
   * @param serverAddr: address of grpc server
   * @param serverPort: port of grpc server
   * @return true - success; false - fail
   */
  virtual bool init(int coreId, std::string serverAddr, int serverPort, Callback *cb) = 0;

  /**
   * @brief implement tcpXfer function in BSP module
   * @param targetChipId: ID of chip
   * @param targetCoreId: ID of target core(spike)
   * @param targetAddr: address of target
   * @param data: address of data
   * @param dataSize: size of data
   * @param streamDir: direction of stream, default is CORE2CORE
   * @param lut: index of lookup table for boradcast, default is disabled
   * @return true - success; false - fail
   */
  virtual bool tcpXfer(uint16_t targetChipId, uint16_t targetCoreId, uint32_t targetAddr,
                    char *data, uint32_t dataSize, uint32_t sourceAddr,
                    StreamDir streamDir = CORE2CORE,
                    uint32_t column = 0, uint32_t dstStride = 0, uint32_t srcStride = 0) = 0;

  /**
   * @brief implement dmaXfer function
   * @param ddrAddr: address of target
   * @param llbAddr: address of source
   * @param streamDir: direction of dma
   * @return true - success; false - fail
   */
  virtual bool dmaXfer(uint64_t dstaddr, uint64_t srcaddr, DmaDir dir,
    uint32_t column, uint32_t len, uint32_t srcStride) = 0;

  /**
   * @brief implement dmaXferPoll function
   * @return true - busy; false - done
   */
  virtual bool dmaPoll() = 0;

  virtual bool llbLoad(uint32_t addr, size_t len, uint8_t* bytes,
                    uint32_t column = 0, uint32_t dstStride = 0, uint32_t srcStride = 0) = 0;
  virtual bool llbStore(uint32_t addr, size_t len, const uint8_t* bytes,
                    uint32_t column = 0, uint32_t dstStride = 0, uint32_t srcStride = 0) = 0;
  virtual bool ddrLoad(uint64_t addr, size_t len, uint8_t* bytes) = 0;
  virtual bool ddrStore(uint64_t addr, size_t len, const uint8_t* bytes) = 0;

  /**
   * @brief implement sync function in BSP module
   * @return true - success; false - fail
   */
  virtual bool sync() = 0;

 protected:
  Callback *mCb;
  int mCoreId;  // ID of cores(pike)
};
}

#endif  // TRANSPORT_ABSTRACT_PROXY_H
