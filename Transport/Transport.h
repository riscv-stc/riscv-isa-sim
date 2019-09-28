/**
 * See LICENSE for license details
 *
 * @author Pascal Ouyang
 * 
 * This file is included to register all transports.
 *
 */
#ifndef TRANSPORT_TRANSPORT_H
#define TRANSPORT_TRANSPORT_H

#include "Factory.h"
#include "GrpcProxy.h"
#include "UdpLogger.h"

namespace Transport {

// register transports to factory
static FactoryRegister<AbstractProxy, GrpcProxy> __grpc_transport("grpc", true);
static FactoryRegister<AbstractLogger, UdpLogger> __udp_transport("udp", true);

}

#endif