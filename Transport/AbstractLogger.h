/**
 * See LICENSE for license details
 *
 * @author Pascal Ouyang
 *
 */

#ifndef TRANSPORT_ABSTRACT_LOGGER_H
#define TRANSPORT_ABSTRACT_LOGGER_H

#include <string>

/**
 * Transport namespace packages entire Transport module
 */
namespace Transport {

/**
 * @brief abstract interface of Transport
 */
class AbstractLogger {
public:
  // pure virtual deconstructor, used for abstract class
  virtual ~AbstractLogger() = 0;

  /**
   * @brief initialize transport
   * @param coreId: ID of core(spike)
   * @param serverAddr: address of grpc server
   * @param serverPort: port of grpc server
   * @return true - success; false - fail
   */
  virtual bool init(int coreId, std::string serverAddr, int serverPort) = 0;

  /**
   * @brief send log message to log server
   * @param data: start address of log message
   * @param size: size of log message
   * @return true - success; false - fail
   */
  virtual bool console(char *data, uint32_t size) = 0;
protected:
  int mCoreId;  // ID of cores(pike)
};
}

#endif  // TRANSPORT_ABSTRACT_LOGGER_H
