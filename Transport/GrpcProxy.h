/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#ifndef TRANSPORT_GRPC_TRANSPORT_H
#define TRANSPORT_GRPC_TRANSPORT_H

#include <grpcpp/grpcpp.h>
#include <mutex>
#include <thread>
#include "proxy.grpc.pb.h"

#include "AbstractProxy.h"

namespace Transport {
using proxy::Proxy;

/**
 * @brief grpc framework
 */
class GrpcProxy : public AbstractProxy {
 public:
  /**
   * @brief initialize grpc framework
   * @param coreId: ID of core(spike)
   * @param serverAddr: address of grpc server
   * @param serverPort: port of grpc server
   * @return true - success; false - fail
   */
  bool init(int coreId, std::string serverAddr, int serverPort, Callback *cb) override;

  /**
   * @brief implement tcpXfer function of BSP module
   * @param targetChipId: ID of chip
   * @param targetCoreId: ID of target core(spike)
   * @param targetAddr: address of target
   * @param data: address of data
   * @param dataSize: size of data
   * @param streamDir: direction of stream, default is CORE2CORE
   * @param lut: index of lookup table for boradcast, default is disabled
   * @return true - success; false - fail
   */
  bool tcpXfer(uint16_t targetChipId, uint16_t targetCoreId, uint32_t targetAddr, char *data,
            uint32_t dataSize, uint32_t sourceAddr, StreamDir streamDir) override;

  /**
   * @brief implement dmaXfer function
   * @param targetAddr: address of target
   * @param sourceAddr: address of source
   * @param streamDir: direction of dma
   * @param len: size of data
   * @return true - success; false - fail
   */
  bool dmaXfer(uint64_t ddrAddr, uint32_t llbAddr, uint32_t len, DmaDir dir) override;

  /**
   * @brief implement dmaXferPoll function
   * @return true - busy; false - done
   */
  bool dmaPoll() override;

  bool ddrLoad(uint64_t addr, size_t len, uint8_t* bytes) override;
  bool ddrStore(uint64_t addr, size_t len, const uint8_t* bytes) override;

  /**
   * @brief implement sync function of BSP module
   * @return true - success; false - fail
   */
  bool sync() override;

 private:
  std::string serverAddr = "";  // address + port of grpc server
  std::shared_ptr<::grpc::Channel> mChn = nullptr;

  std::unique_ptr<Proxy::Stub> proxy() { return Proxy::NewStub(mChn); };

  /**
   * @brief receive data from grpc server and store them in message queue
   */
  void loadToRecvQueue(void);
  std::thread mRecvThread;  // thread to run loadToRecvQueue

#define DEFAULT_DUMP_ADDR 0x10000000
#define DEFAULT_DUMP_SIZE 0x100
#define INVALID_DUMP_VAL -1
  /**
   * @brief dump a block of memory in target
   * @param addr: start address of memory to dump
   * @param size: size of memory to dump
   * @param syncCount: count of done sync, default is -1
   * @return true - success; false - fail
   */
  bool dump(uint32_t addr, uint32_t size, int32_t syncCount = -1);

  /**
   * @brief wait for grpc request to dump memory in target
   */
  void waitDumpRequest(void);
  std::thread mDumpMemThread;  // thread to run loadToRecvQueue

  bool mSyncDumpFlag = false;  // flag to dump memory when sync done
  uint32_t mSyncDumpAddr;      // start of memory to dump when sync done
  uint32_t mSyncDumpSize;      // length of memory to dump when sync done
  std::mutex mDumpMutex;       // mutex for above data

  uint32_t mSyncCount = 0;  // count of done sync
};

}

#endif  // TRANSPORT_GRPC_TRANSPORT_H
