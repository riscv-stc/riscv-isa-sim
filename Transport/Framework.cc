/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#include "Framework.h"
#include "FrameworkGrpc.h"
#include "FrameworkUdpLog.h"
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
                     int grpcServerPort, std::string logServerAddr,
                     int logServerPort) {
  bool ret = false;

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
  if (grpcClient) {
    ret = grpcClient->init(coreId, grpcServerAddr, grpcServerPort);
  }
  if (ret == false) return ret;

  // initialize udp(log) framework
  auto logClient =
      dynamic_cast<FrameworkUdpLog*>(Framework::getInstance(FRAMEWORK_UDP_LOG));
  if (logClient) {
    ret = logClient->init(coreId, logServerAddr, logServerPort);
  }
  if (ret == false) return ret;

  return ret;
}

/**
 * initialize grpc framework
 */
bool Framework::init(uint16_t coreId, std::string serverAddr, int serverPort) {
  return true;
}
