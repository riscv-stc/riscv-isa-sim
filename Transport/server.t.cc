/**
 * See LICENSE for license details
 * Transportsrc=../Transport/
 *  export PKG_CONFIG_PATH=/home/simon/opt/riscv/lib/pkgconfig/ && g++ -g -O0
 * -I${Transportsrc} -I. -std=c++11 ${Transportsrc}/server.t.cc libTransport.a
 * `pkg-config --libs protobuf` `pkg-config --libs grpc++` `pkg-config --cflags
 * grpc` `pkg-config --libs protobuf` -o testServer
 *
 * @author Jiang,Bin
 *
 */

#include <grpcpp/grpcpp.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <string>
#include "proxy.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using dspike::proto::Message;
using proxy::Proxy;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

bool bSent = false;
int count = 0;
// Logic and data behind the server's behavior.
class ServerContextServiceImpl final : public Proxy::Service {
  Status TCPXfer(ServerContext* context, const ::dspike::proto::Message* request,
              ::google::protobuf::Empty* response) override {
    std::cout << "TCPXfer--source ID:" << request->source()
              << " target ID:" << request->target()
              << " target ID:" << request->target()
              << " body:" << request->body() << std::endl;
    printf(" src address:%p, dst address:%p direction:%d\n" ,request->srcaddr(),request->dstaddr(), request->direction());
    bSent = true;
    return Status::OK;
  }

  Status DMAXfer(ServerContext* context, const ::dspike::proto::DMAXferRequest* request,
              ::google::protobuf::Empty* response) override {
    printf("DMAXfer ddr address:%p, llb address:%p direction:%d\n" ,request->ddraddr(),request->llbaddr(),request->direction());
    bSent = true;
    return Status::OK;
  }

  Status TCPXferCb(ServerContext* context,
              grpc::ServerReaderWriter< ::dspike::proto::Message, ::proxy::TCPXferCbRequest>*
                  stream) override {
    proxy::TCPXferCbRequest request;
    stream->Read(&request);
    while (1) {
      while (!bSent)
        ;
      if (count++ > 2) {
        break;
      } else {
    std::cout << "TCPXferCb" << std::endl;
        dspike::proto::Message msg;
        msg.set_body("recv:" + std::to_string(count));
#if 0
        msg.set_source(12);
        msg.set_target(1);
#endif
        msg.set_dstaddr(0xcdfc);
        msg.set_direction(Message::llb2core);
        stream->Write(msg);
        stream->Read(&request);
      }
    }
    return Status::OK;
  }

  Status Sync(ServerContext* context, const ::proxy::SyncRequest* request,
              ::google::protobuf::Empty* response) override {
    std::cout << "Sync" << std::endl;
    sleep(3);
    bSent = false;
    count = 0;
    return Status::OK;
  }
};

/**
 * @brief RunServer
 */
void RunServer() {
  std::string server_address("localhost:3291");
  ServerContextServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main() {
  RunServer();
  return 0;
}
