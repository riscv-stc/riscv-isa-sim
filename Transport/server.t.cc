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
using proxy::Message;
using proxy::RecvRequest;
using proxy::Proxy;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

bool bSent = false;
int count = 0;
// Logic and data behind the server's behavior.
class ServerContextServiceImpl final : public Proxy::Service {
  Status Send(ServerContext* context, const ::proxy::Message* request,
              ::google::protobuf::Empty* response) override {
    std::cout << "Send--source ID:" << request->source()
              << " target ID:" << request->target()
              << " body:" << request->body() << std::endl;
    bSent = true;
    return Status::OK;
  }

  Status Recv(ServerContext* context, const ::proxy::RecvRequest* request,
              grpc::ServerWriter<::proxy::Message>* writer) override {
    std::cout << "Recv--source ID:" << request->spikeid() << std::endl;
    while (1) {
      while (!bSent)
        ;
      if (count++ > 1000) {
        break;
      } else {
        proxy::Message msg;
        msg.set_body("recv:" + std::to_string(count));
        msg.set_source(12);
        msg.set_target(1);
        writer->Write(msg);
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
  std::string server_address("localhost:50051");
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
