/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#ifndef TRANSPORT_FRAMEWORK_H
#define TRANSPORT_FRAMEWORK_H

#include <map>
#include "Interface.h"

namespace Transport {

/**
 * @brief framework of Transport module
 */
class Framework : public Interface {
 public:
  virtual ~Framework();

  /**
   * @brief get singleton instance of a framework
   * @param frameworkType: type of framework
   * @return pointer to interface of a framework
  */
  static Interface *getInstance(FrameworkType frameworkType);

  /**
   * @brief framework map
   * key: type of framework
   * value: pointer to interface a framework
   */
  static std::map<FrameworkType, Interface *> gFrameworkMap;

  /**
   * @brief register a framework to framework map
   * @param frameworkType: type of framework
   * @param instance: pointer to interface of a framwork
   * @return true - success; false - fail
   */
  bool registerInstance(FrameworkType frameworkType, Interface *instance);

  /**
   * @brief initialize framework
   * @param coreId: ID of core(spike)
   * @param sim: pionter to sim_t
   * @param grpcServerAddr: address of grpc server
   * @param grpcServerPort: port of grpc server
   * @return true - success; false - fail
   */
  static bool init(uint16_t coreId, SIM_S *sim, std::string grpcServerAddr,
                   int grpcServerPort);
  /**
   * @brief initialize grpc framework
   * @param coreId: ID of core(spike)
   * @param serverAddr: address of grpc server
   * @param serverPort: port of grpc server
   * @return true - success; false - fail
   */
  virtual bool init(uint16_t coreId, std::string serverAddr, int serverPort);
};
}
#endif  // TRANSPORT_FRAMEWORK_H
