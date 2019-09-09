/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#ifndef TRANSPORT_FRAMEWORK_GRPC_H
#define TRANSPORT_FRAMEWORK_GRPC_H

#include <grpcpp/grpcpp.h>
#include <mutex>
#include <thread>
#include "Framework.h"
#include "Stream.h"
#include "proxy.grpc.pb.h"

namespace Transport {
using proxy::Proxy;

/**
 * @brief grpc framework
 */
class FrameworkGrpc : public Framework {
 public:
  static FrameworkGrpc *gGrpcClient;  // singleton instance of this class

  /**
   * @brief initialize grpc framework
   * @param coreId: ID of core(spike)
   * @param serverAddr: address of grpc server
   * @param serverPort: port of grpc server
   * @return true - success; false - fail
   */
  bool init(uint16_t coreId, std::string serverAddr, int serverPort) override;

  /**
   * @brief implement tcpXfer function of BSP module
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
  bool tcpXfer(uint16_t targetChipId, uint16_t targetCoreId, uint32_t targetAddr, char *data,
            int dataSize, StreamType streamType, StreamDir streamDir, uint16_t tag,
            uint8_t lut) override;

  /**
   * @brief implement dmaXfer function
   * @param targetAddr: address of target
   * @param sourceAddr: address of source
   * @param streamDir: direction of dma
   * @param len: size of data
   * @return true - success; false - fail
   */
  bool dmaXfer(uint32_t targetAddr, uint32_t sourceAddr, DmaDir dir, uint16_t len) override;

  /**
   * @brief implement dmaXferPoll function
   * @return true - busy; false - done
   */
  bool dmaXferPoll() override;

  /**
   * @brief implement sync function of BSP module
   * @param streamType: type of stream
   * @return true - success; false - fail
   */
  bool sync(StreamType streamType) override;

  /**
   * @brief garbage collection to clean gGrpcClient
   */
  class Gc {
   public:
    ~Gc() {
      if (gGrpcClient != nullptr) {
        delete gGrpcClient;
      }
    }
  };

  static Gc gc;

 private:
  std::string serverAddr = "";  // address + port of grpc server
  std::unique_ptr<Proxy::Stub> mTcpXferStub = nullptr;  // stub for tcp Xfer service
  std::unique_ptr<Proxy::Stub> mTcpXferCbStub = nullptr;  // stub for tcp Xfer callback service
  std::unique_ptr<Proxy::Stub> mDmaXferStub = nullptr;  // stub for dma Xfer service
  std::unique_ptr<Proxy::Stub> mDmaXferPollStub = nullptr;  // stub for dma xfer poll service
  std::unique_ptr<Proxy::Stub> mSyncStub = nullptr;  // stub for sync service
  std::unique_ptr<Proxy::Stub> mDumpStub = nullptr;  // stub for dump service
  std::unique_ptr<Proxy::Stub> mWaitDumpStub =
      nullptr;  // stub for wait dump service

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

#endif  // TRANSPORT_FRAMEWORK_GRPC_H
