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
#include <iostream>
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
  PCIE_SOCKET_UPDATE_STATUS
};

enum so_command_code{
  SOCKET_CONNET_UPDATE,
  
};

#define PS_DATA_SIZE_MAX  1024
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


// 配置路由表
struct cfg_route_table{
  so_command_code  cmdWord;
  size_t    emu_board_id;
  uint64_t  board_addr_start;
  uint64_t  board_addr_size;
  in_addr_t  ipaddr;
  uint16_t  port_num;
};

// 连接配置表
struct con_cfg_table{
  size_t    emu_board_id;
  uint64_t  board_addr_start;
  uint64_t  board_addr_size;
  bool link_status; //true up
  struct sockaddr_in sevrAddr;
  int server_connect_fd;
};

class pcie_socket_sim_t : public abstract_device_t {
 public:
  pcie_socket_sim_t(simif_t* sim, size_t board_id, \
      uint8_t board_connect_id, const char * filename);

  ~pcie_socket_sim_t();

  void process_data(void);
  bool load(reg_t addr, size_t len, uint8_t* bytes);
  bool store(reg_t addr, size_t len, const uint8_t* bytes);
  size_t size() { return NPU_CONNECT_CONFIG_IRAM_SIZE; }
  int send_data(reg_t addr, size_t length, uint8_t *data, uint8_t target_id);
 private:
  std::queue<struct pcie_socket_packet > psp_rv_buf;
  std::queue<struct pcie_socket_packet > psp_se_buf;
  std::queue<struct cfg_route_table > cfg_route_que;
  int multicast_serv_fd = -1;
  int multicast_client_fd = -1;
  int server_fd = -1;
  int npu_client_fd = -1;
  in_addr_t ipaddr ;
  simif_t *sim;
  in_addr_t local_server_ip;
  struct sockaddr_in mult_addr;
  struct sockaddr_in tcp_serv_addr;
  std::mutex psp_rv_mutex;
  std::mutex route_table_mutex;
  std::mutex cfg_msg_mutex;
  uint8_t board_connect_id;
  uint16_t logic_board_id = 0;
  uint8_t *reg_base = nullptr;
  uint16_t local_port_number = 0; //socket 服务端绑定的端口
  size_t board_id;
  uint32_t msg_head_id; // 底8位为board_id，之后8位为组id,最后16位为逻辑id
  std::vector<struct con_cfg_table> route_table; //每次路由表更新都要向其他连接广播
  std::vector<struct serv_cfg> vec_serv_cfg;
  bool param_cfg_flag = false;  //该标志起作用则路由将不起作用
  std::thread *sr_t = nullptr;
  int create_server_connfd(int &server_fd, bool multicast, uint16_t *co_port);
  void socket_multicast_server();
  void socket_server(size_t board_id, uint8_t board_connect_id, bool ser_prot);
  int  socket_client(in_addr_t ipaddr, uint16_t port_num, bool ser_prot, 
      struct pcie_socket_packe *se_buffer, uint16_t data_len);
  
  std::string getFilenameFromPath(const std::string& path);
  std::string getExecutableName(const std::string& pid);
  std::string find_program_using_port(int port);
  bool load_data(reg_t addr, size_t len, uint8_t* bytes);
  bool store_data(reg_t addr, size_t len, const uint8_t* bytes);
  void getlocalipaddr(in_addr_t &ipaddr);
  std::string get_current_prgram_name();
  void creat_sender_fd(in_addr_t ipaddr, uint16_t port_number, 
    bool multicast);
  int socket_server_recv(int socket_fd, void *cfg, uint16_t len);
  int sender(int socket_fd, void *msg, uint16_t len, struct sockaddr* sevrAddr, 
             unsigned int sockaddr_len);

  void multicast_recv(void);
  void pcie_socket_recv(void);
  int send_server_cfg();
  bool read_file_cfg(const char *filename);
  void tcp_server_recv();
  void data_socket_process(int new_socket, pcie_socket_packet &buffer, uint32_t buf_len);
};

#endif //__PCIE_SOCKET_SIM_H__