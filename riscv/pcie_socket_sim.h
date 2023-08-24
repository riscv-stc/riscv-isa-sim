#ifndef __PCIE_SOCKET_SIM_H__
#define __PCIE_SOCKET_SIM_H__
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "simif.h"
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <string>
#include <ifaddrs.h>
#include <netdb.h>
#include <fstream>
#include <sstream>

#define NPU_CONNECT_CONFIG_IRAM_START 0xc1070000
#define NPU_CONNECT_CONFIG_IRAM_SIZE  0x10000

#define PCIE_SOCKET_OK 0
#define PCIE_SOCKET_ERR -1
#define PCIE_SOCKET_CONNECT_ERR -2
#define PCIE_SOCKET_DATE_READ_ERR -3
#define PCIE_SOCKET_DATE_SEND_ERR -4
#define PCIE_SOCKET_DATE_WRITE_ERR -5
#define PCIE_SOCKET_BIND_ERR -6
#define PCIE_SOCKET_GETNAME_ERR -7

enum ps_command_code{
  PCIE_SOCKET_WRITE_DDR,
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

class pcie_socket_sim_t : public abstract_device_t {
 public:
  pcie_socket_sim_t(simif_t* sim, size_t board_id, \
      uint8_t board_connect_id);

  ~pcie_socket_sim_t();
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  size_t size() { return NPU_CONNECT_CONFIG_IRAM_SIZE; }
  int send_data(reg_t addr, size_t length, uint8_t *data, uint8_t target_id);
 private:
  simif_t *sim;
  const uint32_t magicWord = 0x12345678;
  int server_fd = -1;
  size_t board_id;
  uint8_t board_connect_id;
  uint8_t *reg_base = nullptr;
  uint16_t local_port_number = 0; //socket 服务端绑定的端口

  in_addr_t local_server_ip;
  struct sockaddr_in tcp_serv_addr;

  std::vector<struct serv_cfg> vec_serv_cfg;
  std::thread *sr_t = nullptr;

  void socket_server(bool ser_prot, struct sockaddr_in *tcp_serv, int &server_fd);
  bool load_data(reg_t addr, size_t len, uint8_t* bytes);
  bool store_data(reg_t addr, size_t len, const uint8_t* bytes);
  void getlocalipaddr(in_addr_t &ipaddr);
  bool read_sock_cfg(char *data, uint16_t len);
  void tcp_server_recv();
  void socket_process(int client_socket, pcie_socket_packet &buffer);
  int socket_client_connet(const char* ipaddr, uint16_t port_num);
  void reload_cfg(bool reload);
  void get_route_cfg();
  void write_soc_ddr(int new_socket, pcie_socket_packet &buffer);
  void npu_reset_cluster(int client_socket, pcie_socket_packet &buffer);
  void cfg_update(int client_socket, pcie_socket_packet &buffer);
  void packet_fill(pcie_socket_packet &bsend, ps_command_code ps);
  int pack_recive_head_check(int fd, struct pcie_socket_packet &bsend, int group_id, 
        uint64_t board_id, ps_command_code ps);
};

#endif //__PCIE_SOCKET_SIM_H__