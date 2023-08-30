#ifndef __PCIE_ROUTE_H__
#define __PCIE_ROUTE_H__

#include <string>
#include <memory>
#include <iostream>

#define PCIE_SOCKET_OK 0
#define PCIE_SOCKET_ERR -1
#define PCIE_SOCKET_CONNECT_ERR -2
#define PCIE_SOCKET_DATE_READ_ERR -3
#define PCIE_SOCKET_DATE_SEND_ERR -4
#define PCIE_SOCKET_DATE_WRITE_ERR -5
#define PCIE_SOCKET_BIND_ERR -6
#define PCIE_SOCKET_GETNAME_ERR -7
#define PCIE_SOCKET_SEND_SERVER_ERR -8
#define PCIE_SOCKET_SERVER_DONE -9
#define SERVER_PORT 0xb000
enum ps_command_code{
  PCIE_SOCKET_WRITE_DDR_START,
  PCIE_SOCKET_WRITE_DDR_STOP,
  PCIE_SOCKET_WRITE_DDR_ERR,
  PCIE_SOCKET_READ_DDR,
  PCIE_SOCKET_SEND_ERR,
  PCIE_SOCKET_SEND_OK,
  PCIE_SOCKET_SEND_RESEND,
  PCIE_SOCKET_RECIVE_OK,
  PCIE_SOCKET_RECIVE_RESEND,
  PCIE_SOCKET_RECIVE_GROID_ERR,
  PCIE_SOCKET_RECIVE_BOAID_ERR,
  PCIE_SOCKET_UPDATE_STATUS,
  PCIE_SOCKET_RESET,
  PCIE_SOCKET_CONFIG_UPDATE,
  PCIE_SOCKET_MAGICE_ERR,
  PCIE_SOCKET_RECIVE_ERR,

  SOCKET_HEART_BEAT = 0x1000,
  SOCKET_DATA_UPDATE,
  SOCKET_CONNECT_REGISTER, // 仅最开始建立连接时候使用，用于server与spike通信
  SOCKET_CONNECT_CFG,
  NPU_RESET_CLUSTER,
  SOFT_EXEC,
  SOFT_EXEC_START,
  SOFT_EXEC_STOP,
  CFG_UPDATE_CLEAR_VCF,
  CFG_UPDATE,
  CFG_UPDATE_START,
  CFG_UPDATE_STOP,
};

#define PS_DATA_SIZE_MAX  512
struct pcie_socket_packet{
  uint32_t  packetHead;
  uint64_t  board_id;
  ps_command_code  cmdWord;
  uint16_t  dataLen;
  uint64_t  addr;
  uint8_t   group_id;
  char data[PS_DATA_SIZE_MAX];
};

struct serv_cfg {
    uint64_t id;
    uint16_t port;
    std::string ip;
};
const uint32_t magicWord = 0x12345678;
#endif