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
                     int grpcServerPort) {
  return Framework::init(coreId, sim, grpcServerAddr, grpcServerPort);
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
                     int dataSize, StreamType streamType, uint8_t lut) {
  return false;
};

/**
 * implement sync function in BSP module
 */
bool Interface::sync(StreamType streamType) { return false; };
