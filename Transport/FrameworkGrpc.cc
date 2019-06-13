/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#include "FrameworkGrpc.h"
#include <iostream>
#include "Interface.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using proxy::Message;
using proxy::RecvRequest;
using proxy::Proxy;

using namespace Transport;

FrameworkGrpc* FrameworkGrpc::gGrpcClient = nullptr;
FrameworkGrpc::Gc FrameworkGrpc::gc;

/*
 * create gGrpcClient singleton and register to framework map
 */
static __attribute__((constructor)) void REGISTER_INSTANCE() {
  if (FrameworkGrpc::gGrpcClient == nullptr) {
    try {
      FrameworkGrpc::gGrpcClient = new FrameworkGrpc;
      FrameworkGrpc::gGrpcClient->registerInstance(Interface::FRAMEWORK_GRPC,
                                                   FrameworkGrpc::gGrpcClient);
    } catch (std::bad_alloc& e) {
      std::cout << "fail to alloc:" << e.what();
    }
  }
}

/**
 * initialize grpc framework
 */
bool FrameworkGrpc::init(uint16_t coreId, std::string serverAddr,
                         int serverPort) {
  gGrpcClient->mCoreId = coreId;
  gGrpcClient->serverAddr = serverAddr + ":" + std::to_string(serverPort);

  gGrpcClient->mSendStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mRecvStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mSyncStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  if (gGrpcClient->mSendStub == nullptr || gGrpcClient->mRecvStub == nullptr ||
      gGrpcClient->mSyncStub == nullptr) {
    std::cout << "failt to new stub" << std::endl;
    return false;
  }

  // start a thread to receive data from grpc server
  auto func = std::bind(&FrameworkGrpc::loadToRecvQueue, gGrpcClient);
  gGrpcClient->mRecvThread = std::thread(func);
  gGrpcClient->mRecvThread.detach();

  return true;
}

/**
 * implement send function of BSP module
 */
bool FrameworkGrpc::send(uint16_t targetChipId, uint16_t targetCoreId,
                         char* data, int dataSize, StreamType streamType,
                         uint8_t lut) {
  if (gGrpcClient->mSendStub == nullptr) {
    std::cout << "send stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }

  // prepare message data
  proxy::Message request;
  request.set_source(mCoreId);
  request.set_target(targetCoreId);
  request.set_forwarding(true);  // TODO, it's useful?
  request.set_body(data, dataSize);
  // container for the data we expect from the server
  google::protobuf::Empty reply;

  // context for the client
  ClientContext context;

  // actual RPC
  Status status = mSendStub->Send(&context, request, &reply);

  if (!status.ok())
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;

  return status.ok();
}

/**
 * receive data from grpc server and store them in message queue
 */
void FrameworkGrpc::loadToRecvQueue(void) {
  if (gGrpcClient->mRecvStub == nullptr) {
    std::cout << "recv stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }
  // only message stream has recv queue
  StreamType streamType = StreamType::STREAM_MESSAGE;

  proxy::RecvRequest request;
  request.set_spikeid(mCoreId);

  proxy::Message reply;

  // context for the client
  ClientContext context;

  // actual RPC
  std::unique_ptr<grpc::ClientReader<Message>> reader(
      mRecvStub->Recv(&context, request));
  while (reader->Read(&reply)) {
    auto stream = Stream::getInstance(streamType);
    // enqueue data to message queue

    if (reply.target() != mCoreId) {
      std::cout << "message is sent to " << reply.target() << " nor " << mCoreId
                << std::endl;
      continue;
    }
    if (stream &&
        stream->recvPost(reply.source(), reply.mutable_body()->data(),
                         reply.mutable_body()->size(), streamType) == false) {
      std::cout << "fail to post receive message" << std::endl;
    }
  }
  Status status = reader->Finish();

  if (status.ok()) {
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

/**
 * implement send function of BSP module
 */
bool FrameworkGrpc::sync(StreamType streamType) {
  if (gGrpcClient->mSyncStub == nullptr) {
    std::cout << "sync stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }
  proxy::SyncRequest request;
  request.set_spikeid(mCoreId);

  google::protobuf::Empty reply;

  // context for the client
  ClientContext context;

  // actual RPC.
  Status status = mSyncStub->Sync(&context, request, &reply);

  if (!status.ok())
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;

  return status.ok();
}
