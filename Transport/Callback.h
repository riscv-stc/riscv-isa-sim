/**
 * See LICENSE for license details
 *
 * @author Pascal Ouyang
 *
 */

#ifndef TRANSPORT_CALLBACK_H
#define TRANSPORT_CALLBACK_H

#include <string>

/**
 * Transport namespace packages entire Transport module
 */
namespace Transport {

/**
 * @brief abstract interface of Transport callback
 */
class Callback {
 public:
  // Receive data from Transport module
  virtual bool recv(uint32_t dstaddr, const char* data, uint32_t size, bool set_active) = 0;
  virtual bool dump(std::string *data, uint64_t addr, uint32_t size) = 0;
};
}

#endif  // TRANSPORT_CALLBACK_H
