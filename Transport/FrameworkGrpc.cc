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

  gGrpcClient->mDumpStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mWaitDumpStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  if (gGrpcClient->mSendStub == nullptr || gGrpcClient->mRecvStub == nullptr ||
      gGrpcClient->mDumpStub == nullptr ||
      gGrpcClient->mWaitDumpStub == nullptr ||
      gGrpcClient->mSyncStub == nullptr) {
    std::cout << "failt to new stub" << std::endl;
    return false;
  }

  // start a thread to receive data from grpc server
  auto func = std::bind(&FrameworkGrpc::loadToRecvQueue, gGrpcClient);
  gGrpcClient->mRecvThread = std::thread(func);
  gGrpcClient->mRecvThread.detach();

  // start a thread to dump memory of target
  auto funcDump = std::bind(&FrameworkGrpc::waitDumpRequest, gGrpcClient);
  gGrpcClient->mDumpMemThread = std::thread(funcDump);
  gGrpcClient->mDumpMemThread.detach();

  return true;
}

/**
 * implement send function of BSP module
 */
bool FrameworkGrpc::send(uint16_t targetChipId, uint16_t targetCoreId,
                         char* data, int dataSize, StreamType streamType,
                         uint8_t lut, uint16_t tag, uint16_t mark) {
  if (gGrpcClient->mSendStub == nullptr) {
    std::cout << "send stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }

  // prepare message data
  proxy::Message request;
  request.set_source(mCoreId);
  request.set_target(targetCoreId);
  request.set_forwarding(false);
  request.set_body(data, dataSize);
  request.set_tag(tag);
  request.set_mark(mark);
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
    return;
  }
  // only message stream has recv queue
  StreamType streamType = StreamType::STREAM_MESSAGE;

  proxy::RecvRequest request;
  request.set_spikeid(mCoreId);

  proxy::Message reply;

  // context for the client
  ClientContext context;

  // actual RPC
  std::unique_ptr<grpc::ClientReaderWriter<RecvRequest, Message>> readWriter(
      mRecvStub->Recv(&context));

  readWriter->Write(request);

  while (readWriter->Read(&reply)) {
    auto stream = Stream::getInstance(streamType);
    // enqueue data to message queue

    if (reply.target() != mCoreId) {
      std::cout << "message is sent to " << reply.target() << " nor " << mCoreId
                << std::endl;
      readWriter->Write(request);
      continue;
    }
    if (stream &&
        stream->recvPost(reply.source(), reply.mutable_body()->data(),
                         reply.mutable_body()->size(), streamType) == false) {
      std::cout << "fail to post receive message" << std::endl;
    }

    readWriter->Write(request);
  }
  Status status = readWriter->Finish();

  if (!status.ok()) {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

/**
 * implement sync function of BSP module
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

  mSyncCount++;

  // dump memory
  if (mSyncDumpFlag) {
    uint32_t addr;
    uint32_t size = 0;
    {
      std::lock_guard<std::mutex> lock(mDumpMutex);
      if (mSyncDumpFlag) {
        addr = mSyncDumpAddr;
        size = mSyncDumpSize;
      }
    }
    if (size != 0) addr = dump(addr, size, mSyncCount);
  }

  if (!status.ok())
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;

  return status.ok();
}

/**
 * dump a block of memory in target
 */
bool FrameworkGrpc::dump(uint32_t addr, uint32_t size, int32_t syncCount) {
  if (gGrpcClient->mDumpStub == nullptr) {
    std::cout << "Dumpd stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }

  // prepare message data
  proxy::DumpData request;
  auto info = new dspike::proto::DumpInfo;
  info->set_spikeid(mCoreId);
  info->set_synccount(syncCount);
  info->set_start(addr);
  info->set_length(size);
  request.set_allocated_info(info);

  auto data = request.mutable_data();
  // dump memory into data
  auto stream = Stream::getInstance(STREAM_DUMP);
  stream->dump(addr, size, data);

  // container for the data we expect from the server
  google::protobuf::Empty reply;

  // context for the client
  ClientContext context;

  // actual RPC
  Status status = mDumpStub->Dump(&context, request, &reply);

  if (!status.ok())
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;

  return status.ok();
}

/**
 * wait for grpc request to dump memory in target
 */
void FrameworkGrpc::waitDumpRequest(void) {
  while (1) {
    proxy::WaitDumpRequest request;
    request.set_spikeid(mCoreId);
    dspike::proto::DumpParam reply;

    // context for the client
    ClientContext context;

    // actual RPC
    std::unique_ptr<grpc::ClientReader<dspike::proto::DumpParam>> reader(
        mWaitDumpStub->WaitDump(&context, request));
    while (reader->Read(&reply)) {
      // request to dump in done sync
      if (reply.trigger_case() == dspike::proto::DumpParam::kOnsync) {
        std::lock_guard<std::mutex> lock(mDumpMutex);
        mSyncDumpFlag = reply.onsync();
        mSyncDumpAddr = reply.start() == INVALID_DUMP_VAL ? DEFAULT_DUMP_ADDR
                                                          : reply.start();
        mSyncDumpSize = reply.length() == INVALID_DUMP_VAL ? DEFAULT_DUMP_SIZE
                                                           : reply.length();
      } else if (reply.trigger_case() == dspike::proto::DumpParam::kNow) {
        // request to dump now
        dump(reply.start() == INVALID_DUMP_VAL ? DEFAULT_DUMP_ADDR
                                               : reply.start(),
             reply.length() == INVALID_DUMP_VAL ? DEFAULT_DUMP_SIZE
                                                : reply.length());
      } else {
        std::cout << "dump reply is empty" << std::endl;
      }
    }
    Status status = reader->Finish();

    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      break;
    }
  }
}
