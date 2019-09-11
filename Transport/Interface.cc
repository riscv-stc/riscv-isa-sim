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
bool Interface::init(uint16_t coreId, SIM_S *sim, std::string grpcServerAddr,
                     int grpcServerPort, std::string logServerAddr,
                     int logServerPort)
{
  return Framework::init(coreId, sim, grpcServerAddr, grpcServerPort,
                         logServerAddr, logServerPort);
}

/**
 * get singleton instance of a framework
 */
Interface *Interface::getInstance(FrameworkType frameworkType)
{
  return Framework::getInstance(frameworkType);
}

/**
 * implement tcpXfer function in BSP module
 */
bool Interface::tcpXfer(uint16_t targetChipId, uint16_t targetCoreId,
                     uint32_t targetAddr, char *data, int dataSize, uint32_t sourceAddr,
                     StreamDir streamDir, StreamType streamType, uint16_t tag, uint8_t lut)
{
  return false;
};

/**
 * implement dmaXfer function
 */
bool Interface::dmaXfer(uint64_t ddrAddr, uint32_t llbAddr, DmaDir dir, uint16_t len)
{
  return false;
};

/**
 * implement dmaXferPoll function
 */
bool Interface::dmaXferPoll()
{
  return false;
};

/**
 * implement sync function in BSP module
 */
bool Interface::sync(StreamType streamType) { return false; };

/**
 * send log message to log server
 */
bool Interface::console(char *data, uint32_t size) { return false; };
