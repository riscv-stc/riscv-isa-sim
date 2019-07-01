/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#include "Framework.h"

using namespace Transport;

Interface::~Interface() {}

/**
 * initialize Transport module
 */
bool Interface::init(uint16_t coreId, SIM_S* sim, std::string grpcServerAddr,
                     int grpcServerPort, std::string logServerAddr,
                     int logServerPort) {
  return Framework::init(coreId, sim, grpcServerAddr, grpcServerPort,
                         logServerAddr, logServerPort);
}

/**
 * get singleton instance of a framework
 */
Interface* Interface::getInstance(FrameworkType frameworkType) {
  return Framework::getInstance(frameworkType);
}

/**
 * implement send function in BSP module
 */
bool Interface::send(uint16_t targetChipId, uint16_t targetCoreId, char* data,
                     int dataSize, StreamType streamType, uint16_t tag,
                     uint16_t mark, uint8_t lut) {
  return false;
};

/**
 * implement sync function in BSP module
 */
bool Interface::sync(StreamType streamType) { return false; };

/**
 * send log message to log server
 */
bool Interface::console(char* data, uint32_t size) { return false; };
