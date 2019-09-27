/**
 * See LICENSE for license details
 *
 * @author Jiang,Bin
 *
 */

#include "Interface.h"

using namespace Transport;

Interface::~Interface() {}

/**
 * implement init function in BSP module
 */
bool Interface::init(int coreId, std::string serverAddr, int serverPort, Callback *cb) {

}

/**
 * implement tcpXfer function in BSP module
 */
bool Interface::tcpXfer(uint16_t targetChipId, uint16_t targetCoreId,
                     uint32_t targetAddr, char *data, uint32_t dataSize, uint32_t sourceAddr,
                     StreamDir streamDir)
{
  return false;
};

/**
 * implement dmaXfer function
 */
bool Interface::dmaXfer(uint64_t ddrAddr, uint32_t llbAddr, uint32_t len, DmaDir dir, char *data)
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
bool Interface::sync() { return false; };

/**
 * send log message to log server
 */
bool Interface::console(char *data, uint32_t size) { return false; };
