/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#include "FrameworkUdpLog.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

using namespace Transport;

FrameworkUdpLog *FrameworkUdpLog::gUdpLogClient = nullptr;
FrameworkUdpLog::Gc FrameworkUdpLog::gc;

/*
 * create gUdpLogClient singleton and register to framework map
 */
static __attribute__((constructor)) void REGISTER_INSTANCE() {
  if (FrameworkUdpLog::gUdpLogClient == nullptr) {
    try {
      FrameworkUdpLog::gUdpLogClient = new FrameworkUdpLog;
      FrameworkUdpLog::gUdpLogClient->registerInstance(
          Interface::FRAMEWORK_UDP_LOG, FrameworkUdpLog::gUdpLogClient);
    } catch (std::bad_alloc &e) {
      std::cout << "fail to alloc:" << e.what();
    }
  }
}

FrameworkUdpLog::~FrameworkUdpLog() {
  if (mSockFd >= 0) close(mSockFd);
}
/**
 * initialize udp framework
 */
bool FrameworkUdpLog::init(uint16_t coreId, std::string serverAddr,
                           int serverPort) {
  gUdpLogClient->mCoreId = coreId;

  /* build udp socket */
  mSockFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (mSockFd < 0) {
    std::cout << "udp sock fails to create" << std::endl;
    return false;
  }

  /* set address */
  memset(&mSockAddr, 0, sizeof(mSockAddr));
  mSockAddr.sin_family = AF_INET;
  mSockAddr.sin_addr.s_addr = inet_addr(serverAddr.data());
  mSockAddr.sin_port = htons(serverPort);
  mSockAddrLen = sizeof(mSockAddr);

  return false;
}

/**
 * send log message to log server
 */
bool FrameworkUdpLog::console(char *data, uint32_t size) {
  if (mSockFd < 0) return false;

  if (size > 0) {
    std::string message = "logs,spike_id=" + std::to_string(mCoreId) +
                          " message=\"" + std::string(data, size) + "\"";
    int sent = sendto(mSockFd, message.data(), message.size(), 0,
                      (struct sockaddr *)&mSockAddr, mSockAddrLen);
    if (message.size() != sent)
      std::cout << "sendto fails, sent:" << sent << "size:" << size
                << std::endl;
  }
  return true;
}
