/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#include "StreamDumpMem.h"
#include <string.h>
#include <iostream>

using namespace Transport;

StreamDumpMem *StreamDumpMem::gStreamDumpMem = nullptr;

/*
 * create gStreamDumpMem singleton and register to stream map
 */
static __attribute__((constructor)) void REGISTER_INSTANCE() {
  if (StreamDumpMem::gStreamDumpMem == nullptr) {
    try {
      StreamDumpMem::gStreamDumpMem = new StreamDumpMem;
      StreamDumpMem::gStreamDumpMem->registerInstance(
          Interface::STREAM_MESSAGE, StreamDumpMem::gStreamDumpMem);
    } catch (std::bad_alloc &e) {
      std::cout << "fail to alloc:" << e.what();
    }
  }
}

/*
 * dump memory in target to data
 */
bool StreamDumpMem::dump(uint64_t addr, uint32_t size, std::string *data) {
  data->clear();
  data->append((char *)addr, size);
  return true;
}
