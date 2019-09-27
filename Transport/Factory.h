/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */
#ifndef TRANSPORT_FRAMEWORK_H
#define TRANSPORT_FRAMEWORK_H

#include <unordered_map>
#include <functional>
#include <iostream>
#include "Interface.h"

namespace Transport {

/**
 * @brief Transport factory
 */
class Factory {
 public:
  // use this to instantiate the proper Derived class
  static Interface * createTransport(const std::string& name = "") {
    auto it = Factory::registry().find(name);
    return it == Factory::registry().end() ? nullptr : (it->second)();
  }

  typedef std::unordered_map<std::string, std::function<Interface*()>> registry_map;

  static registry_map& registry() {
    static registry_map impl;
    return impl;
  }
};

template<typename T> struct FactoryRegister
{
    FactoryRegister(std::string name, bool is_default = false)
    {
      Factory::registry()[name] = []() { return new T; };
      if (is_default)
        Factory::registry()[""] = []() { return new T; };
      std::cout << "transport class '" << name << (is_default?"(default)":"") << "' registered.\n";
    }
};

} // namespace Transport

#endif  // TRANSPORT_FRAMEWORK_H
