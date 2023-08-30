#ifndef __PCIE_SOCKET_SIM_H__
#define __PCIE_SOCKET_SIM_H__
#include <vector>
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
#include "pcie_route.h"
#define NPU_CONNECT_CONFIG_IRAM_START 0xc1070000
#define NPU_CONNECT_CONFIG_IRAM_SIZE  0x10000
struct connect_cfg{
    struct serv_cfg sc;
    int fd;
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
  int close_connect_fd(reg_t addr, uint8_t target_id);
 private:
  simif_t *sim;
  int server_fd = -1;
  size_t board_id;
  uint8_t board_connect_id;
  uint8_t *reg_base = nullptr;
  uint16_t local_port_number = 0; //socket 服务端绑定的端口

  in_addr_t local_server_ip;
  struct sockaddr_in tcp_serv_addr;

  std::vector<struct connect_cfg > vec_con_cfg;
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
  int write_soc_ddr(int new_socket, pcie_socket_packet &buffer);
  void npu_reset_cluster(int client_socket, pcie_socket_packet &buffer);
  void cfg_update(int client_socket, pcie_socket_packet &buffer);
  void packet_fill(pcie_socket_packet &bsend, ps_command_code ps);
  int pack_recive_head_check(int fd, struct pcie_socket_packet &bsend, int group_id, 
        uint64_t board_id, ps_command_code ps);

  void thread_write_soc(int client_socket);
  int get_connect_fd(struct connect_cfg &temp);
  void close_all_connect_fd();
};

#endif //__PCIE_SOCKET_SIM_H__