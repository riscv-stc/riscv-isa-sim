/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <string>

/**
 * Transport namespace packages entire Transport module
 */
namespace Transport {

typedef void SIM_S;

/**
 * @brief abstract interface of Transport framework
 */
class Interface {
 public:
  // type of framework
  enum FrameworkType {
    FRAMEWORK_GRPC = 0,  // grpc
    FRAMEWORK_UDP_LOG,   // raw udp for logger
  };

  // type of stream used in GRPC framework
  enum StreamType {
    // public for user
    STREAM_MESSAGE = 0,  // message
    STREAM_RDMA    = 1,  // rdma

    // private for user
    STREAM_DUMP = 100,  // dump memory
  };

  // direction of stream used in GRPC framework
  enum StreamDir {
    CORE2CORE  = 0,  // from a core(spike) to another core(spike)
    CORE2LLB   = 1,  // from a core(spike) to LLB
    LLB2CORE   = 2,  // from LLB to a core(spike)
  };

  // direction of DMA
  enum DmaDir {
    LLB2DDR  = 0,   // from LLB to DDR
    DDR2LLB   = 1,  // from DDR to LLB
  };

  // pure virtual deconstructor, used for abstract class
  virtual ~Interface() = 0;

  /**
   * @brief initialize Transport module
   * @param coreId: ID of core(spike)
   * @param sim: pionter to sim_t
   * @param grpcServerAddr: address of grpc server, default is localhost
   * @param grpcServerPort: port of grpc server, default is 3291
   * @param logServerAddr: address of log server, default is 127.0.0.1
   * @param logServerPort: port of log server, default is 3291
   * @return true - success; false - fail
   */
  static bool init(uint16_t coreId, SIM_S *sim,
                   std::string grpcServerAddr = "localhost",
                   int grpcServerPort = 3291,
                   std::string logServerAddr = "127.0.0.1",
                   int logServerPort = 3291);

  /**
   * @brief get singleton instance of a framework
   * @param frameworkType: type of framework, default is grpc framework
   * @return pointer to interface of a framework
   */
  static Interface *getInstance(FrameworkType frameworkType = FRAMEWORK_GRPC);

#define LUT_DISABLE 0xff  // invalid lut means lut is not used

  /**
   * @brief implement tcpXfer function in BSP module
   * @param targetChipId: ID of chip
   * @param targetCoreId: ID of target core(spike)
   * @param targetAddr: address of target
   * @param data: address of data
   * @param dataSize: size of data
   * @param streamType: type of stream, default is STREAM_MESSAGE
   * @param streamDir: direction of stream, default is CORE2CORE
   * @param lut: index of lookup table for boradcast, default is disabled
   * @return true - success; false - fail
   */
  virtual bool tcpXfer(uint16_t targetChipId, uint16_t targetCoreId, uint32_t targetAddr,
                    char *data, uint32_t dataSize, uint32_t sourceAddr, StreamDir streamDir = CORE2CORE,
                     StreamType streamType = STREAM_MESSAGE, uint16_t tag = 0, uint8_t lut = LUT_DISABLE);

  /**
   * @brief implement dmaXfer function
   * @param ddrAddr: address of target
   * @param llbAddr: address of source
   * @param streamDir: direction of dma
   * @param len: size of data
   * @return true - success; false - fail
   */
  virtual bool dmaXfer(uint64_t ddrAddr, uint32_t llbAddr, DmaDir dir, uint32_t len);

  /**
   * @brief implement dmaXferPoll function
   * @return true - busy; false - done
   */
  virtual bool dmaXferPoll();

  /**
   * @brief implement sync function in BSP module
   * @param streamType: type of stream, defalut is STREAM_MESSAGE
   * @return true - success; false - fail
   */
  virtual bool sync(StreamType streamType = STREAM_MESSAGE);

  /**
   * @brief send log message to log server
   * @param data: start address of log message
   * @param size: size of log message
   * @return true - success; false - fail
   */
  virtual bool console(char *data, uint32_t size);

 protected:
  int mCoreId;  // ID of cores(pike)
};
}

#endif  // TRANSPORT_INTERFACE_H
