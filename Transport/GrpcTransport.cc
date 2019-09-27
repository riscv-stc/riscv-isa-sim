/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#include <iostream>
#include "Interface.h"

#include "GrpcTransport.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using dspike::proto::Message;
using dspike::proto::DMAXferPollResponse;
using proxy::RecvCbRequest;
using proxy::Proxy;

using namespace Transport;

/**
 * initialize grpc framework
 */
bool GrpcTransport::init(int coreId, std::string serverAddr, int serverPort,
        Callback *cb) {
  this->mCb = cb;
  this->mCoreId = coreId;
  this->serverAddr = serverAddr + ":" + std::to_string(serverPort);

  this->mTcpXferStub = Proxy::NewStub(grpc::CreateChannel(
      this->serverAddr, grpc::InsecureChannelCredentials()));

  this->mDmaXferStub = Proxy::NewStub(grpc::CreateChannel(
      this->serverAddr, grpc::InsecureChannelCredentials()));

  this->mDmaXferPollStub = Proxy::NewStub(grpc::CreateChannel(
      this->serverAddr, grpc::InsecureChannelCredentials()));

  this->mRecvCbStub = Proxy::NewStub(grpc::CreateChannel(
      this->serverAddr, grpc::InsecureChannelCredentials()));

  this->mSyncStub = Proxy::NewStub(grpc::CreateChannel(
      this->serverAddr, grpc::InsecureChannelCredentials()));

  this->mDumpStub = Proxy::NewStub(grpc::CreateChannel(
      this->serverAddr, grpc::InsecureChannelCredentials()));

  this->mWaitDumpStub = Proxy::NewStub(grpc::CreateChannel(
      this->serverAddr, grpc::InsecureChannelCredentials()));

  if (this->mTcpXferStub == nullptr || this->mRecvCbStub == nullptr ||
      this->mDumpStub == nullptr ||
      this->mWaitDumpStub == nullptr ||
      this->mSyncStub == nullptr) {
    std::cout << "failt to new stub" << std::endl;
    return false;
  }

  // start a thread to receive data from grpc server
  auto func = std::bind(&GrpcTransport::loadToRecvQueue, this);
  this->mRecvThread = std::thread(func);
  this->mRecvThread.detach();

  // start a thread to dump memory of target
  auto funcDump = std::bind(&GrpcTransport::waitDumpRequest, this);
  this->mDumpMemThread = std::thread(funcDump);
  this->mDumpMemThread.detach();

  return true;
}

/**
 * implement tcpXfer function of BSP module
 */
bool GrpcTransport::tcpXfer(uint16_t targetChipId, uint16_t targetCoreId,
                         uint32_t targetAddr, char* data, uint32_t dataSize, uint32_t sourceAddr,
                         StreamDir streamDir) {
  if (this->mTcpXferStub == nullptr) {
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
          request.set_type(Message::rdma);
          request.set_dstaddr(targetAddr);
          break;

    case LLB2CORE:
          request.set_target(targetCoreId);
          request.set_srcaddr(sourceAddr);
          request.set_dstaddr(targetAddr);
          request.set_direction(Message::llb2core);
          request.set_length(dataSize);
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
bool GrpcTransport::dmaXfer(uint64_t ddrAddr, uint32_t llbAddr, uint32_t len, DmaDir dir, char *data) {
  if (this->mDmaXferStub == nullptr) {
    std::cout << "dmaXfer stub is null, since grpc doesn't initialize"
              << std::endl;
    return false;
  }

  // prepare message data
  Message request;
  switch (dir) {
    case LLB2DDR:
          request.set_target(mCoreId);
          request.set_srcaddr(llbAddr);
          request.set_dstaddr(ddrAddr);
          request.set_direction(Message::llb2ddr);
          request.set_length(len);
          break;

    case DDR2LLB:
          request.set_source(mCoreId);
          request.set_body(data, len);
          request.set_dstaddr(llbAddr);
          request.set_direction(Message::ddr2llb);
          break;
    default:
          break;
  }

  request.set_length(len);

  fprintf(stdout, "[dmaXfer] direction:%d ddr addr:0x%lx llb addr:0x%x data size:%d\n",
                        dir, ddrAddr, llbAddr, len);

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
bool GrpcTransport::dmaXferPoll() {
  if (this->mDmaXferPollStub == nullptr) {
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
void GrpcTransport::loadToRecvQueue(void) {
  if (this->mRecvCbStub == nullptr) {
    std::cout << "recv stub is null, since grpc doesn't initialize"
              << std::endl;
    return;
  }

  proxy::RecvCbRequest request;
  request.set_spikeid(mCoreId);

  dspike::proto::Message reply;

  // context for the client
  ClientContext context;

  // actual RPC
  std::unique_ptr<grpc::ClientReaderWriter< proxy::RecvCbRequest, Message>> readWriter(
      mRecvCbStub->RecvCb(&context));

  readWriter->Write(request);

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

    auto xdir = (reply.direction() == Message::llb2core)? StreamDir::LLB2CORE: StreamDir::CORE2CORE;

    if (!mCb->recv(reply.dstaddr(), reply.mutable_body()->data(),
                       reply.mutable_body()->size(), xdir)) {
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
bool GrpcTransport::sync() {
  if (this->mSyncStub == nullptr) {
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
bool GrpcTransport::dump(uint32_t addr, uint32_t size, int32_t syncCount) {
  if (this->mDumpStub == nullptr) {
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
  if (mCb->dump(data, addr, size)) {
    std::cout << "fail to dump memory at" << std::hex << addr << std::endl;
  }

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
void GrpcTransport::waitDumpRequest(void) {
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
