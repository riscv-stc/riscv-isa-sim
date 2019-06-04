/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#ifndef TRANSPORT_FRAMEWORK_GRPC_H
#define TRANSPORT_FRAMEWORK_GRPC_H

#include <grpcpp/grpcpp.h>
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
   * @brief implement send function of BSP module
   * @param targetId: ID of target core(spike)
   * @param data: message data
   * @param dataSize: size of message data
   * @param streamType: type of stream
   * @param lut: index of lookup table for boradcast
   * @return true - success; false - fail
   */
  bool send(uint16_t targetChipId, uint16_t targetCoreId, char *data,
            int dataSize, StreamType streamType, uint8_t lut) override;

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
  std::unique_ptr<Proxy::Stub> mSendStub = nullptr;  // stub for send service
  std::unique_ptr<Proxy::Stub> mRecvStub = nullptr;  // stub for recv service
  std::unique_ptr<Proxy::Stub> mSyncStub = nullptr;  // stub for sync service

  /**
   * @brief receive data from grpc server and store them in message queue
   */
  void loadToRecvQueue(void);

  std::thread mRecvThread;  // thread to run loadToRecvQueue
};
}

#endif  // TRANSPORT_FRAMEWORK_GRPC_H
