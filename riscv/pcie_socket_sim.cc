#include "pcie_socket_sim.h"
#include <algorithm>

#define MAX_BUFFER 1518
#define INIT_PORT_NUMBER 0xc000 //默认端口
#define MAX_SERVER_LISTEN_NUM 256

#define CFG_SERVE_IP "127.0.0.1" // LO
#define CFG_SERVE_PORT 0xb000 
#define SECLET_WAIT_TIME_S    5
bool server_running = true;
pcie_socket_sim_t::pcie_socket_sim_t(simif_t* sim, size_t board_id, \
    uint8_t board_connect_id)
    :sim(sim), board_id(board_id), board_connect_id(board_connect_id){
    
    // 创建服务端，获得系统端口，并传递给服务端用于通信，只用于配置通信。
    reload_cfg(false);
    
    // 创建客户端连接fd
    get_route_cfg();
}

void pcie_socket_sim_t::get_route_cfg(){
    int cfg_client_fd = socket_client_connet(CFG_SERVE_IP, CFG_SERVE_PORT);
    if (cfg_client_fd < 0){
        perror("can't connect cfg server please check!");
        exit(EXIT_FAILURE);
    }

    struct pcie_socket_packet temp;
    packet_fill(temp, SOCKET_CONNECT_REGISTER);
    temp.addr = (local_port_number & 0xFFFF);

    if ( send(cfg_client_fd, (void *)&temp, sizeof(temp), 0) < 0){
        perror("send to cfg server error");
        exit(EXIT_FAILURE);
    }

    bzero(&temp, sizeof(temp));
    if (pack_recive_head_check(cfg_client_fd, temp, board_connect_id, board_id, PCIE_SOCKET_RECIVE_OK) != PCIE_SOCKET_OK){
        std::cout << "spike register in server error!"<< std::endl;
        exit(EXIT_FAILURE);
    }
    
}

// 解析行内容，获得版卡id ip地址，端口号
serv_cfg parseLine(const std::string &line) {
    serv_cfg data;
    std::stringstream ss(line);
    std::string temp;

    while (ss >> temp) {
        if (temp.find("id:") != std::string::npos) {
            data.id = std::stoi(temp.substr(3));
        } else if (temp.find("ip:") != std::string::npos) {
            data.ip = temp.substr(3);
        } else if (temp.find("port:") != std::string::npos) {
            data.port = std::stoi(temp.substr(5));
        }
    }

    return data;
}

// 删除因# //出现的行或者其之后的内容
std::string processLine(const std::string& line) {
    size_t hashPos = line.find('#');
    size_t slashPos = line.find("//");

    // 找到第一个出现的符号
    size_t firstPos = std::min(hashPos, slashPos);

    // 如果找到其中任何一个，裁剪该位置之后的所有内容
    if (firstPos != std::string::npos) {
        return line.substr(0, firstPos);
    }
    return line;  // 如果没有找到，返回原始行
}

// reset 将会重新设置该表
bool pcie_socket_sim_t::read_sock_cfg(char *data, uint16_t len){
    std::istringstream iss(std::string(data, len));
    std::string line;
    while (std::getline(iss, line)) {
        std::string temp = processLine(line);
        
        if (!temp.size())   //经过处理后的行如果为空行则跳过
            continue;
        serv_cfg cfg;
        cfg = parseLine(temp);
        vec_con_cfg.push_back({cfg, -1});   
        if (cfg.id == board_id)
        {
            local_port_number = cfg.port;
            local_server_ip = inet_addr(cfg.ip.c_str());
        }
    }
    return true;
}

void pcie_socket_sim_t::getlocalipaddr(in_addr_t &ipaddr){
    struct ifaddrs *interfaces = nullptr;
    struct ifaddrs *interface = nullptr;

    if (getifaddrs(&interfaces) == -1) {
        std::cerr << "getifaddrs error" << std::endl;
        return;
    }   
    int a = 0;

    for (interface = interfaces; interface != nullptr; interface = interface->ifa_next) {
        if (interface->ifa_addr == nullptr) continue;
        if ((interface->ifa_addr->sa_family == AF_INET)  && (strcmp(interface->ifa_name, "lo") != 0) && (strncmp(interface->ifa_name, "docker", strlen("docker")) != 0)) { // check it is IP4
            // is a valid IP4 Address
            auto *addr = reinterpret_cast<struct sockaddr_in *>(interface->ifa_addr);
            // inet_ntop(AF_INET, &(addr->sin_addr), host, NI_MAXHOST);
            ipaddr = addr->sin_addr.s_addr; //获得第一个通信的ipv4地址,如果名称中存在bond直接结束
            if((a = std::string(interface->ifa_name).find("bond")) > 0)
                break;
        }
    }   

    freeifaddrs(interfaces);
}

void pcie_socket_sim_t::socket_server(bool ser_prot, struct sockaddr_in *tcp_serv, int &server_fd){
    // 创建socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }   

    // 检查该端口是否被占用，如果被占用则报错，并退出
    tcp_serv->sin_family = AF_INET;
    tcp_serv->sin_addr.s_addr = INADDR_ANY;

    if (ser_prot)
        tcp_serv->sin_port = htons(local_port_number);
    else
        tcp_serv->sin_port = 0; // 系统分配
    // 绑定socket到端口
    if (bind(server_fd, (struct sockaddr *)tcp_serv, sizeof(struct sockaddr_in)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }   
    
    // 最多等待256个客户端连接
    if (listen(server_fd, MAX_SERVER_LISTEN_NUM) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }   
    
    if (!ser_prot){
        socklen_t len = sizeof(struct sockaddr_in);
        if (getsockname(server_fd, (struct sockaddr*)tcp_serv, &len) == -1) {
            std::cerr << "Failed to get socket name" << std::endl;
            exit(EXIT_FAILURE);
        }   

        local_port_number = ntohs(tcp_serv->sin_port);
    }
}

void pcie_socket_sim_t::packet_fill(pcie_socket_packet &bsend, ps_command_code ps){
    bzero(&bsend, sizeof(pcie_socket_packet));
    bsend.board_id = board_id;
    bsend.group_id = board_connect_id;
    bsend.cmdWord = ps;
    bsend.packetHead = magicWord;
}

void pcie_socket_sim_t::thread_write_soc(int client_socket){
    pcie_socket_packet bsend;
    bzero(&bsend, sizeof(bsend));
    // struct timeval timout;
    // timout.tv_sec = 2; // 2秒超时则结束线程表示程序结束
    // timout.tv_usec = 0;
    
    while(1){
        // setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timout, sizeof(timout));
        if (recv(client_socket, &bsend, sizeof(bsend), 0) < 0){
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::cout << "recv() timed out." << std::endl;
                // continue; // 超时 重新接收.
            } else {
                perror("recv");
            }
            break; // 超时或出错
        }
        if (bsend.cmdWord != PCIE_SOCKET_WRITE_DDR_STOP){
            if (write_soc_ddr(client_socket, bsend) != PCIE_SOCKET_OK)
                break;
        }
        else{
            bzero(&bsend, sizeof(bsend) - PS_DATA_SIZE_MAX);
            packet_fill(bsend, PCIE_SOCKET_RECIVE_OK);
            if (send(client_socket, &bsend, sizeof(bsend) - PS_DATA_SIZE_MAX, 0) < 0){
                perror("pcie socket send error in end!");
            }
            break;
        }
        bzero(&bsend, sizeof(bsend));
    }
    close(client_socket);
}

int pcie_socket_sim_t::write_soc_ddr(int client_socket, pcie_socket_packet &buffer){
    pcie_socket_packet bsend;
    packet_fill(bsend, PCIE_SOCKET_RECIVE_OK);
    
    if (buffer.board_id != board_id){
        std::cout << "Received error! board id error" << std::endl;
        bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_BOAID_ERR;
    }else if (buffer.board_id != board_connect_id){
        std::cout << "Received error! group id error" << std::endl;
        bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_GROID_ERR;
    }else if (buffer.cmdWord == PCIE_SOCKET_WRITE_DDR_START){
        if (store_data(buffer.addr & 0xFFFFFFFFFF, buffer.dataLen, (uint8_t *)buffer.data)){
            bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_OK;
        }
        else{
            bsend.cmdWord = ps_command_code::PCIE_SOCKET_WRITE_DDR_ERR;
        }
    }else if (buffer.cmdWord == PCIE_SOCKET_WRITE_DDR_STOP){
        bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_OK;
    }else{
        std::cout << "server recve to client msg error!" << std::endl;
        bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_RESEND;
    }
    if (!server_running)
        return PCIE_SOCKET_SERVER_DONE;

    if(send(client_socket, (void *)&bsend, (sizeof(pcie_socket_packet) - PS_DATA_SIZE_MAX), 0) < 0){
        std::cout << "server send to client error!" << std::endl;
    }
    return PCIE_SOCKET_OK;
}

void pcie_socket_sim_t::npu_reset_cluster(int client_socket, pcie_socket_packet &buffer){
    pcie_socket_packet bsend;
    packet_fill(bsend, PCIE_SOCKET_RECIVE_OK);

    close_all_connect_fd();
    //调用 reset函数
    
    for (unsigned i = 0; i < sim->nprocs(); i++){
        processor_t *processor = sim->get_core_by_idxinsim(i);
        processor->reset();
    }
    if ( send(client_socket, (void *)&bsend, sizeof(bsend)- PS_DATA_SIZE_MAX, 0) < 0){
        std::cout << "spike send to manager server of reset error board id is " << board_id <<
            "connect id is " << board_connect_id << std::endl;
    }
}

int pcie_socket_sim_t::pack_recive_head_check(int fd, struct pcie_socket_packet &bsend, int group_id, 
        uint64_t board_id, ps_command_code ps){
    if (recv(fd, (void *)&bsend, sizeof(bsend) - PS_DATA_SIZE_MAX, 0) < 0){
            std::cout << "server send to spike error of cfg update board id" << board_id << std::endl;
            // exit(EXIT_FAILURE);
            return PCIE_SOCKET_RECIVE_ERR;
        }
    if (bsend.packetHead != magicWord){
        std::cout << "server recv from spike packetHead error of cfg update board id " << board_id << std::endl;
        return PCIE_SOCKET_MAGICE_ERR;
    }

    if (bsend.cmdWord != ps){
        std::cout << "server recv from spike cmdWord error of cfg update board id " << board_id << 
            "cmdword error " << bsend.cmdWord << std::endl;
        return PCIE_SOCKET_ERR;
    }

    if (bsend.group_id != group_id){
        std::cout << "server recv from spike group_id error of cfg update board id " << board_id << std::endl;
        return PCIE_SOCKET_RECIVE_GROID_ERR;
    }

    if (bsend.board_id != board_id){
        std::cout << "server recv from spike error of board id " << board_id << std::endl;
        return PCIE_SOCKET_RECIVE_BOAID_ERR;
    }
    return PCIE_SOCKET_OK;
}

void pcie_socket_sim_t::close_all_connect_fd(){
    for (int i = 0; vec_con_cfg.size(); i++){
        if (vec_con_cfg[i].fd != -1)
            close(vec_con_cfg[i].fd);
    }
}

void pcie_socket_sim_t::cfg_update(int client_socket, pcie_socket_packet &buffer){
    pcie_socket_packet bsend;
    packet_fill(bsend, PCIE_SOCKET_RECIVE_OK);
    uint16_t data_len = 0;
    
    if ( send(client_socket, (void *)&bsend, sizeof(bsend) - PS_DATA_SIZE_MAX, 0) < 0){
        std::cout << "spike send to manager server cfg update error board id is " << board_id <<
            "connect id is " << board_connect_id << std::endl;
    }

    char *sock_file = (char *)malloc(buffer.dataLen + 1);
    bzero(sock_file, buffer.dataLen + 1);

    while(data_len < buffer.dataLen){
        bzero(&bsend, sizeof(bsend));
        if (recv(client_socket, (void *)&bsend, sizeof(bsend), 0) < 0){
            std::cout << "spike recv manager server cfg error boadrd id is " << board_id <<
            "connect id is " << board_connect_id << std::endl;
            return;
        }
        
        memcpy(sock_file + data_len, bsend.data, bsend.dataLen);
        data_len += bsend.dataLen;
        packet_fill(bsend, PCIE_SOCKET_RECIVE_OK);
        if (send(client_socket, (void *)&bsend, sizeof(bsend) - PS_DATA_SIZE_MAX, 0) < 0){
            std::cout << "spike send to manager server cfg update error board id is " << board_id <<
            "connect id is " << board_connect_id << std::endl;
            return;
        }
    }
    
    close_all_connect_fd();
    if (!vec_con_cfg.empty())
        vec_con_cfg.clear(); // 清除ip配置，用于重新加载，仅仅清除内容不释放占用的空间
    // 通过更新的配置文件重新建立服务
    read_sock_cfg(sock_file, buffer.dataLen);
    reload_cfg(true);

    bzero(&bsend, sizeof(bsend));
    
    pack_recive_head_check(client_socket, bsend, board_connect_id, board_id, CFG_UPDATE_STOP);
    packet_fill(bsend, PCIE_SOCKET_RECIVE_OK);
    if (send(client_socket, (void *)&bsend, sizeof(bsend) - PS_DATA_SIZE_MAX, 0) < 0){
            std::cout << "spike send to manager server cfg update stop error board id is " << board_id <<
            "connect id is " << board_connect_id << std::endl;
            return;
    }

    close(client_socket);
    if (sock_file != nullptr)
        free(sock_file);
    server_running = false;
}

void pcie_socket_sim_t::socket_process(int client_socket, pcie_socket_packet &buffer){
    // pcie_socket_packet bsend;
    // std::cout << "Received: " << buffer << std::endl;
    std::thread *write_soc_t;
    switch (buffer.cmdWord)
    {
    case PCIE_SOCKET_WRITE_DDR_START:
        write_soc_ddr(client_socket, buffer);
        write_soc_t = new std::thread(&pcie_socket_sim_t::thread_write_soc, this, client_socket);
        write_soc_t->detach();
        break;
    case NPU_RESET_CLUSTER:
        npu_reset_cluster(client_socket, buffer);
        break;
    case CFG_UPDATE:
        cfg_update(client_socket, buffer);
        break;
    default:
        break;
    }
    
}

void pcie_socket_sim_t::tcp_server_recv(){
    int client_socket;
    pcie_socket_packet buffer;
    struct sockaddr_in address, client_address;
    uint32_t buf_len = sizeof(struct pcie_socket_packet);
    uint32_t addrLen = sizeof(tcp_serv_addr);
    fd_set read_fds;
    struct timeval tv;
    tv.tv_sec = SECLET_WAIT_TIME_S;
    tv.tv_usec = 0;
    int maxfd = server_fd;
    while (server_running) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        int activity = select(maxfd + 1, &read_fds, NULL, NULL, &tv);
        if (activity < 0){
            perror("select error");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(server_fd, &read_fds)){
            // std::cout << "Waiting for a connection..." << std::endl;
            if ((client_socket = accept(server_fd, (struct sockaddr *)&tcp_serv_addr, \
                        &addrLen)) < 0) {
                std::cerr << "Accept failed" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (recv(client_socket, (void *)&buffer, sizeof(pcie_socket_packet), 0) < 0){
                std::cout << "Received error! while resent" << std::endl; 
            }
            else{
                socket_process(client_socket, buffer);
            }
            if (buffer.cmdWord != PCIE_SOCKET_WRITE_DDR_START)
                close(client_socket);
            memset(&buffer, 0, buf_len);
        }
        
    } 
    close(maxfd);
    server_running = true;
}

int pcie_socket_sim_t::socket_client_connet(const char* ipaddr, uint16_t port_num){
    
    int client_socket = -1;
    struct sockaddr_in server_addr;
    
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        std::cerr << "Socket creation error" << std::endl;
        return PCIE_SOCKET_ERR; 
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_num);
    
    if (inet_pton(AF_INET, ipaddr, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1; 
    }  
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        return PCIE_SOCKET_CONNECT_ERR; 
    }

    return client_socket;
}

bool pcie_socket_sim_t::load(reg_t addr, size_t len, uint8_t* bytes){
    if( nullptr == reg_base || nullptr == bytes || addr + len >= size())
        return false;

    memcpy((char *)bytes, reg_base + addr, len);
    return true;
}

bool pcie_socket_sim_t::store(reg_t addr, size_t len, const uint8_t* bytes){
    if( nullptr == reg_base || nullptr == bytes || addr + len >= size())
        return false;

    memcpy(reg_base + addr, (char *)bytes, len);
    return true;
}

bool pcie_socket_sim_t::store_data(reg_t addr, size_t len, const uint8_t* bytes)
{
    char *host_addr = nullptr;

    if (nullptr != (host_addr = sim->addr_to_mem(addr))) {
        memcpy(host_addr, bytes, len);
    } else if (!sim->mmio_store(addr, len, bytes)) {
        std::cout << "pcie socket sim store addr: 0x"
            << hex
            << addr
            << " access fault."
            << std::endl;
        throw trap_store_access_fault(false, addr, 0, 0);
    }
    
    return true;
}

bool pcie_socket_sim_t::load_data(reg_t addr, size_t len, uint8_t* bytes)
{
    char *host_addr = nullptr;

    if (nullptr != (host_addr=sim->addr_to_mem(addr))) {
        memcpy(bytes, host_addr, len);
    } else if (!sim->mmio_load(addr, len, bytes)) {
        std::cout << "pcie socket sim load addr: 0x"
            << hex
            << addr
            << " access fault."
            << std::endl;
        throw trap_load_access_fault(false, addr, 0, 0);
    }

    return true;
}

void pcie_socket_sim_t::reload_cfg(bool reload){
    
    socket_server(reload, &tcp_serv_addr, server_fd); //use tcp 

    std::thread sv(&pcie_socket_sim_t::tcp_server_recv, this);
    sv.detach();
}

int pcie_socket_sim_t::get_connect_fd(struct connect_cfg &temp){
    if (temp.fd != -1)
        return temp.fd;
    
    temp.fd = socket_client_connet(temp.sc.ip.c_str(), temp.sc.port);
    
    return temp.fd;
}

int pcie_socket_sim_t::close_connect_fd(reg_t addr, uint8_t target_id){
    int i;

    for( i = 0; vec_con_cfg.size(); i++)
    {
        if (vec_con_cfg[i].sc.id == target_id)
            break;
    }
    if (i == vec_con_cfg.size())
    {
        std::cout << "target npu " << target_id << "not set!" << std::endl;
        return PCIE_SOCKET_ERR;
    }

    if (vec_con_cfg[i].fd != -1){
        struct pcie_socket_packet msg;
        packet_fill(msg, PCIE_SOCKET_WRITE_DDR_STOP);
        msg.board_id = target_id;
        
        if (send(vec_con_cfg[i].fd, (void *)&msg, sizeof(msg) - PS_DATA_SIZE_MAX + msg.dataLen, 0) < 0)
        {
            perror("msg send error, please check network!");
            return PCIE_SOCKET_SEND_ERR;
        }
        bzero(&msg, sizeof(msg) - PS_DATA_SIZE_MAX);
        if (recv(vec_con_cfg[i].fd, (void *)&msg, sizeof(msg) - PS_DATA_SIZE_MAX, 0) < 0){
            std::cout << "write soc stop error!" << std::endl;
        }
        pack_recive_head_check(vec_con_cfg[i].fd, msg, board_connect_id, target_id, PCIE_SOCKET_RECIVE_OK);
        close(vec_con_cfg[i].fd);
        vec_con_cfg[i].fd = -1;
    }

    return PCIE_SOCKET_OK;
}

int pcie_socket_sim_t::send_data(reg_t addr, size_t length, uint8_t *data, uint8_t target_id){
    struct pcie_socket_packet msg;
    int rv = PCIE_SOCKET_OK;
    size_t i;
    int sock_fd;
    packet_fill(msg, PCIE_SOCKET_WRITE_DDR_START);

    msg.addr = addr;
    msg.dataLen = length;
    msg.board_id = target_id;
    memcpy(msg.data, data, length);

    for( i = 0; vec_con_cfg.size(); i++)
    {
        if (vec_con_cfg[i].sc.id == target_id)
            break;
    }
    if (i == vec_con_cfg.size())
    {
        std::cout << "target npu " << target_id << "not set!" << std::endl;
        return PCIE_SOCKET_ERR;
    }

    if ((sock_fd = get_connect_fd(vec_con_cfg[i])) > 0)
    {
        if (send(sock_fd, (void *)&msg, sizeof(msg) - PS_DATA_SIZE_MAX + msg.dataLen, 0) < 0)
        {
            perror("msg send error, please check network!");
            vec_con_cfg[i].fd = -1;
            return PCIE_SOCKET_SEND_ERR;
        }
        bzero(&msg, sizeof(msg) - PS_DATA_SIZE_MAX);
        rv = recv(sock_fd, (void *)&msg, sizeof(msg) - PS_DATA_SIZE_MAX, 0);
        switch(msg.cmdWord){
            case PCIE_SOCKET_RECIVE_OK:
                if (msg.board_id != target_id || msg.group_id != board_connect_id)
                {
                    perror("recv a not match msg!");
                }

                break;
            case PCIE_SOCKET_RECIVE_BOAID_ERR:
                perror("send ip addr or port error!");
                break;
            case PCIE_SOCKET_RECIVE_GROID_ERR:
                perror("send group no match error!");
                break;
            default:
                std::cout << "packethead error,please check it!" << std::endl;
                break;
        }
        
        // close(sock_fd);
    }else{
        std::cout << "connect board " << board_id << " error ip is " << vec_con_cfg[i].sc.ip << " port is"
            << vec_con_cfg[i].sc.port << "please check that!" << std::endl;
        return PCIE_SOCKET_ERR;
    }
    
    return PCIE_SOCKET_OK;
}

pcie_socket_sim_t::~pcie_socket_sim_t(){
    if (reg_base != nullptr)
        free(reg_base);

    if (sr_t != nullptr){
        if (sr_t->joinable())
            sr_t->join();
    }
}