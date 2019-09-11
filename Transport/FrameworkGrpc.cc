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
using dspike::proto::Message;
using dspike::proto::DMAXferRequest;
using dspike::proto::DMAXferPollResponse;
using proxy::TCPXferCbRequest;
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

  gGrpcClient->mTcpXferStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mDmaXferStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mDmaXferPollStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mTcpXferCbStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mSyncStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mDumpStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  gGrpcClient->mWaitDumpStub = Proxy::NewStub(grpc::CreateChannel(
      gGrpcClient->serverAddr, grpc::InsecureChannelCredentials()));

  if (gGrpcClient->mTcpXferStub == nullptr || gGrpcClient->mTcpXferCbStub == nullptr ||
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
 * implement tcpXfer function of BSP module
 */
bool FrameworkGrpc::tcpXfer(uint16_t targetChipId, uint16_t targetCoreId,
                         uint32_t targetAddr, char* data, int dataSize, uint32_t sourceAddr,
                         StreamDir streamDir, StreamType streamType, uint16_t tag, uint8_t lut) {
  if (gGrpcClient->mTcpXferStub == nullptr) {
    std::cout << "tcpXfer stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }

  // prepare message data
  Message request;
  switch (streamDir) {
    case CORE2CORE:
          request.set_source(mCoreId);
          request.set_target(targetCoreId);
          request.set_forwarding(false);
          request.set_body(data, dataSize);
          request.set_tag(tag);
          if (streamType == StreamType::STREAM_MESSAGE)
            request.set_type(Message::message);
          else
            request.set_type(Message::rdma);
          request.set_dstaddr(targetAddr);
          break;

    case LLB2CORE:
          request.set_srcaddr(sourceAddr);
          request.set_dstaddr(targetAddr);
          request.set_direction(Message::llb2core);
          break;

    case CORE2LLB:
          request.set_source(mCoreId);
          request.set_body(data, dataSize);
          request.set_dstaddr(targetAddr);
          request.set_direction(Message::core2llb);
          break;
    default:
          break;
  }

  fprintf(stdout, "[tcpXfer] direction:%d, dst:0x%08x src:0x%08x type:%d data size:%d\n",
                           request.direction(), request.dstaddr(), request.srcaddr(), request.type(), dataSize);

  // container for the data we expect from the server
  google::protobuf::Empty reply;

  // context for the client
  ClientContext context;

  // actual RPC
  Status status = mTcpXferStub->TCPXfer(&context, request, &reply);

  if (!status.ok())
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;

  return status.ok();
}

/**
 * implement dmaXfer function
 */
bool FrameworkGrpc::dmaXfer(uint64_t ddrAddr, uint32_t llbAddr, DmaDir dir, uint16_t len) {
  if (gGrpcClient->mDmaXferStub == nullptr) {
    std::cout << "dmaXfer stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }

  // prepare message data
  DMAXferRequest request;
  request.set_llbaddr(llbAddr);
  request.set_ddraddr(ddrAddr);
  if (dir == LLB2DDR)
    request.set_direction(DMAXferRequest::llb2ddr);
  else
    request.set_direction(DMAXferRequest::ddr2llb);
  request.set_length(len);

  fprintf(stdout, "[dmaXfer] direction:%d ddr addr:0x%lx llb addr:0x%x data size:%d\n",
                        dir, request.ddraddr(), request.llbaddr(), len);

  google::protobuf::Empty reply;

  // context for the client
  ClientContext context;

  // actual RPC
  Status status = mDmaXferStub->DMAXfer(&context, request, &reply);

  if (!status.ok())
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;

  return status.ok();
}

/**
 * implement dmaXferPoll function
 */
bool FrameworkGrpc::dmaXferPoll() {
  if (gGrpcClient->mDmaXferPollStub == nullptr) {
    std::cout << "dmaXferPoll stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }

  google::protobuf::Empty request;

  fprintf(stdout, "[dmaXferPoll]\n");

  // container for the data we expect from the server
  DMAXferPollResponse reply;

  // context for the client
  ClientContext context;

  // actual RPC
  Status status = mDmaXferPollStub->DMAXferPoll(&context, request, &reply);

  if (!status.ok()) {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return true; // when fail, also return busy mode
  }
  else
    return reply.busy();
}

/**
 * receive data from grpc server and store them in message queue
 */
void FrameworkGrpc::loadToRecvQueue(void) {
  if (gGrpcClient->mTcpXferCbStub == nullptr) {
    std::cout << "recv stub is null, since grpc doesn't initialize"
              << std::endl;
    return;
  }

  proxy::TCPXferCbRequest request;
  request.set_spikeid(mCoreId);

  dspike::proto::Message reply;

  // context for the client
  ClientContext context;

  // actual RPC
  std::unique_ptr<grpc::ClientReaderWriter< proxy::TCPXferCbRequest, Message>> readWriter(
      mTcpXferCbStub->TCPXferCb(&context));

  readWriter->Write(request);

  auto stream = Stream::getInstance(StreamType::STREAM_MESSAGE);
  while (readWriter->Read(&reply)) {
    // enqueue data to message queue

    if (reply.direction() == Message::core2core && reply.target() != mCoreId) {
      std::cout << "message is sent to " << reply.target() << " nor " << mCoreId
                << std::endl;
      readWriter->Write(request);
      continue;
    }

    fprintf(stdout, "[tcpXferCb] direction:%d, dst:0x%08x type:%d data size:%d\n",
                         reply.direction(), reply.dstaddr(), reply.type(), reply.mutable_body()->size());
    auto streamType = StreamType::STREAM_MESSAGE;
    if (reply.type() != Message::message) {
      streamType = StreamType::STREAM_RDMA;
    }

    auto streamDir = StreamDir::CORE2CORE;
    if (reply.direction() == Message::llb2core)
      streamDir = StreamDir::LLB2CORE;

    if (stream &&
        stream->recvPost(reply.source(), reply.dstaddr(), reply.mutable_body()->data(),
                       reply.mutable_body()->size(), streamType, streamDir,
                       reply.tag()) == false) {
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
