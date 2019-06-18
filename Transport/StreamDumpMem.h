/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 */

#ifndef TRANSPORT_STREAM_DUMP_MEM_H
#define TRANSPORT_STREAM_DUMP_MEM_H

#include <string>
#include "Stream.h"

namespace Transport {
/**
 * @brief Message stream
 */
class StreamDumpMem : public Stream {
 public:
  static StreamDumpMem* gStreamDumpMem;  // singleton instance of this class

  /**
   * @brief dump memory in target to data
   * @param addr: start address of memory to dump
   * @param size: size of memory to dump
   * @param[out] data: store dumped memory to data
   * @return true - success; false - fail
   */
  bool dump(uint64_t addr, uint32_t size, std::string* data);

  /**
   * @brief garbage collection to clean gStreamDumpMem
   */
  class Gc {
   public:
    ~Gc() {
      if (gStreamDumpMem != nullptr) {
        delete gStreamDumpMem;
      }
    }
  };

 private:
};
}

#endif  // TRANSPORT_STREAM_DUMP_MEM_H
