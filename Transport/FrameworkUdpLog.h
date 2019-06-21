/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#ifndef TRANSPORT_FRAMEWORK_UDP_LOG_H
#define TRANSPORT_FRAMEWORK_UDP_LOG_H

#include <netinet/in.h>
#include <sys/socket.h>
#include "Framework.h"

namespace Transport {

/**
 * @brief udp framework
 */
class FrameworkUdpLog : public Framework {
 public:
  static FrameworkUdpLog *gUdpLogClient;  // singleton instance of this class

  ~FrameworkUdpLog() override;

  /**
   * @brief initialize udp framework
   * @param coreId: ID of core(spike)
   * @param serverAddr: address of log udp server
   * @param serverPort: port of log udp server
   * @return true - success; false - fail
   */
  bool init(uint16_t coreId, std::string serverAddr, int serverPort) override;

  /**
   * @brief send log message to log server
   * @param data: start address of log message
   * @param size: size of log message
   * @return true - success; false - fail
   */
  bool console(char *data, uint32_t size) override;

  /**
   * @brief garbage collection to clean gUdpLogClient
   */
  class Gc {
   public:
    ~Gc() {
      if (gUdpLogClient != nullptr) {
        delete gUdpLogClient;
      }
    }
  };

  static Gc gc;

 private:
  int mSockFd = -1;
  struct sockaddr_in mSockAddr;
  socklen_t mSockAddrLen;
};
}

#endif  // TRANSPORT_FRAMEWORK_UDP_LOG_H
