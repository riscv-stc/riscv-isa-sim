/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#include "Framework.h"
#include "FrameworkGrpc.h"
#include "Stream.h"

using namespace Transport;

Framework::~Framework() {}

std::map<Interface::FrameworkType, Interface*> Framework::gFrameworkMap;

/**
 * get instance of a framework
 */
Interface* Framework::getInstance(FrameworkType frameworkType) {
  return gFrameworkMap[frameworkType];
}

/**
 * register an framework and store it in framework map
 */
bool Framework::registerInstance(FrameworkType frameworkType,
                                 Interface* instance) {
  gFrameworkMap[frameworkType] = instance;
  return true;
}

/**
 * initialize framework
 */
bool Framework::init(uint16_t coreId, SIM_S* sim, std::string grpcServerAddr,
                     int grpcServerPort) {
  bool ret;

  // initialize stream
  ret = Stream::init(static_cast<sim_t*>(sim));
  if (ret == false) return ret;

  /*
   * initialize grpc framework
   * Following dynamic cast is FrameworkGrpc, nor Framework for LD link.
   * If use Framework, should set LDFLAGS with
   * "with -Wl,--whole-archive  -lTransport -Wl,--no-whole-archive"
   */
  auto grpcClient =
      dynamic_cast<FrameworkGrpc*>(Framework::getInstance(FRAMEWORK_GRPC));
  ret = grpcClient->init(coreId, grpcServerAddr, grpcServerPort);
  if (ret == false) return ret;

  return ret;
}

/**
 * initialize grpc framework
 */
bool Framework::init(uint16_t coreId, std::string serverAddr, int serverPort) {
  return true;
}
