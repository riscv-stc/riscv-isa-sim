#include "pcie_socket_sim.h"
#include <algorithm>

#define MAX_BUFFER 1518
#define INIT_PORT_NUMBER 0xc000 //默认端口
#define MAX_SERVER_LISTEN_NUM 8

#define MULTICAST_IP "239.255.255.250"
#define MULTICAST_PORT 0xb000 //多播端口

pcie_socket_sim_t::pcie_socket_sim_t(simif_t* sim, size_t board_id, \
    uint8_t board_connect_id, const char * filename)
    :sim(sim), board_connect_id(board_connect_id),
    board_id(board_id){
    
    #if 0
    std::string result;
    msg_head_id = ((board_id & 0xFF) | (board_connect_id << 8));
    reg_base = (uint8_t *)malloc(NPU_CONNECT_CONFIG_IRAM_SIZE);
    uint16_t mult_port = MULTICAST_PORT;
    // 创建用于发现和通知拓扑结构的UDP服务端，所有进程只有第一个创建该服务端,该服务为多播端口
    if ((result = find_program_using_port(MULTICAST_PORT)).empty()){
        if (create_server_connfd(multicast_serv_fd, true, &mult_port) != PCIE_SOCKET_OK)
            multicast_serv_fd = -1;
        else{
            // 多播服务端作用，用于监听受到的数据包，数据包中包含ip地址,端口号，用于为其分配的版卡ID。
            std::thread multicast_serv_t(&pcie_socket_sim_t::multicast_recv, this);
            multicast_serv_t.detach();
        }
    }
    else{
        //获取当前程序名称并与绑定端口的程序作比较是否一致，如果不一致则认为其他程序占用该端口报错，并退出。
        if (get_current_prgram_name() != result){
            std::cout << "multicast port been bind check that port " << mult_port <<std::endl;
            exit(EXIT_FAILURE);
        }    
    }
    // 获得本机ipv4地址
    getlocalipaddr(ipaddr);
    
    // 创建本地通信服务端，并随机产生服务端口,如果出错则表示无法获取服务端口号，则退出
    create_server_connfd(npu_serv_fd, false, &port_number);
    
    // 创建多播发送端fd，之后将本地服务端口号和ip地址发送到多播服务中
    creat_sender_fd(inet_addr(MULTICAST_IP), MULTICAST_PORT, true);

    // 该服务将解决端口号和ip地址需要设置的问题。
    // 当出现本进程的分配逻辑版卡id发生变化则需要通过该udp更新状态。
    // 与其他服务端建立连接后将会保留连接的socket_fd。
    // 由于采用UDP，可能出现连接异常或者数据丢失问题，因此增加重传机制，传输失败1s后重新发送
    // 并记录重传次数，如果超过3次失败，则重新与该连接建立关系，并更新socket_fd。
    //1.socket以及基本通信资源初始化
    sr_t = new std::thread(&pcie_socket_sim_t::pcie_socket_recv, this);
    sr_t->detach();

    if (send_server_cfg() != PCIE_SOCKET_OK)
    {
        std::cout<< "send server ip " << ipaddr << " port " << port_number << 
            " to multicast server error!" << std::endl;
    }
    //2.等待信号允许通信
    #endif

    read_file_cfg(filename);
    socket_server(board_id, board_connect_id, true); //use tcp 
    std::thread sv(&pcie_socket_sim_t::tcp_server_recv, this);
    sv.detach();
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
int pcie_socket_sim_t::read_file_cfg(const char *filename){
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string temp = processLine(line);
        
        if (!temp.size())   //经过处理后的行如果为空行则跳过
            continue;
        serv_cfg cfg;
        cfg = parseLine(temp);
        vec_serv_cfg.push_back(cfg);   
        if (cfg.id == board_id)
        {
            port_number = cfg.port;
            local_server_ip = inet_addr(cfg.ip.c_str());
        }
    }
    file.close();
}

// 这个是需要的时候再配置，可能需要根据HPE的配置来修改以及放置到信息处理位置
int pcie_socket_sim_t::send_server_cfg(){
    struct cfg_route_table cfg;
    int rv = PCIE_SOCKET_OK;
    int i = 0;
    cfg.cmdWord = SOCKET_CONNET_UPDATE;
    cfg.board_addr_size = 0;        // 此时还不曾知道地址大小和起始位置
    cfg.board_addr_start = 0;
    cfg.emu_board_id = ((board_id & 0xFF) | (board_connect_id << 8) | (logic_board_id << 16)) ;
    cfg.ipaddr = ipaddr;
    cfg.port_num = port_number;

    while(i < 3){
        rv = sender(multicast_client_fd, &cfg, sizeof(mult_addr), (struct sockaddr*)&mult_addr, 
            sizeof(mult_addr));
        if (rv != PCIE_SOCKET_OK)
            i++;
        else
            break;
    }
    return rv;
}

// 处理cfg_route_que则在execute中
void pcie_socket_sim_t::multicast_recv(void){
    struct cfg_route_table cfg;
    uint16_t len = sizeof(cfg);
    while(true){
        memset(&cfg, 0, len);
        if (socket_server_recv(multicast_serv_fd, (void *)&cfg, len) == PCIE_SOCKET_OK){
            std::unique_lock<std::mutex> lock(cfg_msg_mutex);
            cfg_route_que.push(cfg);
        }
    }
}

// void pcie_socket_sim_t::pcie_socket_recv(void){
//     struct pcie_socket_packet cfg;
//     uint16_t len = sizeof(cfg);
//     while(true){
//         memset(&cfg, 0, len);
//         if (socket_server_recv(npu_serv_fd, (void *)&cfg, len) == PCIE_SOCKET_OK){
//             std::unique_lock<std::mutex> lock(psp_rv_mutex);
//             psp_rv_buf.push(cfg);
//         }
//     }
// }

std::string pcie_socket_sim_t::get_current_prgram_name(){
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    std::string program_name;
    if (len != -1) {
        buffer[len] = '\0';
        std::string path(buffer);
        program_name = path.substr(path.find_last_of("/") + 1); 
        // std::cout << "The current program name is: " << program_name << std::endl;
    } else {
        std::cerr << "Failed to get program name." << std::endl;
        exit(EXIT_FAILURE);
    }   
    return program_name;
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

// 查询端口是否被spike程序占用
std::string pcie_socket_sim_t::find_program_using_port(int port) {
    std::string command = "lsof -t -i :" + std::to_string(port);
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }   
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }   

    if (!result.empty()) {
        std::stringstream ss(result);
        std::string line;
    
        if( std::getline(ss, line)) {
            std::string processName = getExecutableName(line);
            // std::cout << "processname is " << processName << std::endl;
            result = getFilenameFromPath(processName);
        }
    }

    return result; 
}

std::string pcie_socket_sim_t::getExecutableName(const std::string& pid) {
    std::string cmd = "readlink -f /proc/" + pid + "/exe";
    std::array<char, 512> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    result.erase(std::remove_if(result.begin(), result.end(), [](char c) \
        { return std::isspace(c); }), result.end());

    return result;
}

std::string pcie_socket_sim_t::getFilenameFromPath(const std::string& path) {
    std::string::size_type pos = path.rfind('/');
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

int pcie_socket_sim_t::create_server_connfd(int &server_fd, bool multicast, uint16_t *co_port){
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in localSock;
    memset(&localSock, 0, sizeof(localSock));
    localSock.sin_addr.s_addr = INADDR_ANY;
    localSock.sin_family = AF_INET;

    localSock.sin_port = htons(*co_port); // 如果为0 则由系统分配端口

    // 绑定socket到端口
    if (bind(server_fd, (struct sockaddr *)&localSock, sizeof(localSock)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        if (*co_port == MULTICAST_PORT)
            return PCIE_SOCKET_BIND_ERR; //绑定出错可能是多播端口被绑定
        else
            exit(EXIT_FAILURE);
    }   

    if (multicast){
        struct ip_mreq group;
        group.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
        group.imr_interface.s_addr = inet_addr("0.0.0.0");
        if (setsockopt(server_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&group, sizeof(group)) < 0) {
            perror("adding multicast group failed");
            exit(EXIT_FAILURE);
        }
    }
    else{
        socklen_t len = sizeof(localSock);
        if (getsockname(server_fd, (struct sockaddr*)&localSock, &len) == -1) {
            std::cerr << "Failed to get socket name" << std::endl;
            exit(EXIT_FAILURE);
        }   

        *co_port = ntohs(localSock.sin_port);
    }

    return PCIE_SOCKET_OK;
}

int pcie_socket_sim_t::socket_server_recv(int socket_fd, void *cfg, uint16_t len){
    int rv;
    struct sockaddr_in client_address;
    rv = recvfrom(socket_fd, 
                 (void *)cfg, 
                 len, 
                 0, 
                 (struct sockaddr*)&client_address, 
                 (socklen_t *)sizeof(client_address));
    // 返回出错信息
    if (rv < 0) {
        perror("server read failed");
        return PCIE_SOCKET_RECIVE_RESEND;
    }
    return PCIE_SOCKET_OK;
}

void pcie_socket_sim_t::creat_sender_fd(in_addr_t ipaddr, uint16_t port_number, bool multicast) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("client socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (multicast){
        memset(&mult_addr, 0, sizeof(mult_addr));
        mult_addr.sin_family = AF_INET;
        mult_addr.sin_port = port_number;
        mult_addr.sin_addr.s_addr = ipaddr;
        multicast_client_fd = socket_fd;
    }
    else{
        struct con_cfg_table temp;
        memset(&temp, 0, sizeof(temp));
        temp.sevrAddr.sin_addr.s_addr = ipaddr;
        temp.sevrAddr.sin_port = port_number;
        temp.sevrAddr.sin_family = AF_INET;
        temp.server_connect_fd = socket_fd;
        temp.link_status = true;
        std::unique_lock<std::mutex> lock(route_table_mutex);
        route_table.push_back(temp);
    }
}

int pcie_socket_sim_t::sender(int socket_fd, void *msg, uint16_t len, struct sockaddr* sevrAddr, 
                                unsigned int sockaddr_len){

    if (sendto(socket_fd, 
              msg, 
              len, 
              0, 
              sevrAddr, 
              sockaddr_len) < 0) {

        perror("sendto failed");
        return PCIE_SOCKET_DATE_SEND_ERR;
    }
    
    return PCIE_SOCKET_OK;
}

// 创建两套方案 udp/tcp，两者之间做取舍默认采用udp
void pcie_socket_sim_t::socket_server(size_t board_id, uint8_t board_connect_id, bool ser_prot){
    int new_socket;
    struct sockaddr_in address, client_address;
    int addrlen = sizeof(address);
    int i;
    uint16_t listen_port = 0;
    pcie_socket_packet buffer, bsend;
    int buf_len = sizeof(struct pcie_socket_packet);
    memset(&buffer, 0, sizeof(struct pcie_socket_packet));
    
    // 创建socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket failed" << std::endl;
        exit(EXIT_FAILURE);
    }   


    // 检查该端口是否被占用，如果被占用则报错，并退出
    listen_port = port_number;
    tcp_serv_addr.sin_family = AF_INET;
    tcp_serv_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_serv_addr.sin_port = htons(listen_port);
    // 绑定socket到端口
    if (bind(server_fd, (struct sockaddr *)&tcp_serv_addr, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }   
    
 
    // 最多等待8个客户端连接
    if (listen(server_fd, MAX_SERVER_LISTEN_NUM) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }   
    
}

void pcie_socket_sim_t::tcp_server_recv(){
    int new_socket;
    pcie_socket_packet buffer, bsend;
    struct sockaddr_in address, client_address;
    int buf_len = sizeof(struct pcie_socket_packet);
    uint32_t addrLen = sizeof(tcp_serv_addr);
    bsend.board_id = board_id;
    bsend.group_id = board_connect_id;
    while (1) {
        // std::cout << "Waiting for a connection..." << std::endl;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&tcp_serv_addr, \
                    &addrLen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            exit(EXIT_FAILURE);
        }
        // std::cout << "Received: " << buffer << std::endl;
        if (read(new_socket, (void *)&buffer, buf_len) < 0){
            std::cout << "Received error! while resent" << std::endl; 
            bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_RESEND;
        }
        else if (buffer.board_id != board_id){
            std::cout << "Received error! board id error" << std::endl;
            bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_BOAID_ERR;
        }else if (buffer.board_id != board_id){
            std::cout << "Received error! group id error" << std::endl;
            bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_GROID_ERR;
        }else if (buffer.cmdWord == PCIE_SOCKET_WRITE_DDR){
            uint8_t *soc_addr = (uint8_t*)sim->addr_to_mem((reg_t)(buffer.addr & 0xFFFFFFFFFF));//将地址转换为本地地址
            if (soc_addr != NULL){
                bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_OK;
                memcpy((void *)soc_addr, buffer.data, buffer.dataLen);
            }
            else{
                bsend.cmdWord = ps_command_code::PCIE_SOCKET_WRITE_DDR_ERR;
            }

        }else{
            std::cout << "server recve to client msg error!" << std::endl;
            bsend.cmdWord = ps_command_code::PCIE_SOCKET_RECIVE_RESEND;
        }
        if(send(new_socket, (void *)&bsend, (buf_len - PS_DATA_SIZE_MAX), 0) < 0){
            std::cout << "server send to client error!" << std::endl;
        }
        close(new_socket);
        memset(&buffer, 0, buf_len);
    } 
}

int pcie_socket_sim_t::socket_client(in_addr_t ipaddr, uint16_t port_num, 
            bool ser_prot, struct pcie_socket_packe *se_buffer, uint16_t data_len){
    int sock = 0;
    struct sockaddr_in serv_addr;
    int val = 0;
    socklen_t addr_len = sizeof(serv_addr);
    
    if (ser_prot){
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Socket creation error" << std::endl;
            return PCIE_SOCKET_ERR; 
        }   
    }
    else{
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            std::cerr << "Socket creation error" << std::endl;
            return PCIE_SOCKET_ERR; 
        }   
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num);
    serv_addr.sin_addr.s_addr = ipaddr;  

    if (ser_prot){
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            return PCIE_SOCKET_CONNECT_ERR; 
        }   

        if(send(sock, (void *)se_buffer, data_len, 0) < 0){
            std::cout << "send data error!" << std::endl;
            return PCIE_SOCKET_DATE_SEND_ERR;
        }
        
        if (read(sock, (void *)se_buffer, MAX_BUFFER) < 0 ){
            std::cout << "read back error!" << std::endl;
            return PCIE_SOCKET_DATE_READ_ERR;
        }   
    }
    else{
        val = sendto(sock, 
              (void *)se_buffer, 
              data_len, 
              0, 
              (struct sockaddr*)&serv_addr, addr_len);
        if (val < 0){
            std::cout << "send data error!" << std::endl;
            return PCIE_SOCKET_DATE_SEND_ERR;
        }
        std::cout << "Data sent" << std::endl;
        val = recvfrom(sock, (void *)se_buffer, MAX_BUFFER, 0, NULL, NULL);   
        if (val < 0){
            std::cout << "send data error!" << std::endl;
            return PCIE_SOCKET_DATE_READ_ERR;
        }
    }
    return PCIE_SOCKET_OK;
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

void pcie_socket_sim_t::process_data(void)
{
    {
        uint8_t *data = nullptr;
        std::unique_lock<std::mutex> lock(psp_rv_mutex);
        while(!psp_rv_buf.empty()) {
            auto pCmd = psp_rv_buf.front();
            lock.unlock();
            switch (pCmd.cmdWord) {
            case PCIE_SOCKET_READ_DDR:
                data = (uint8_t *)malloc(PS_DATA_SIZE_MAX);
                memset(data, 0, pCmd.dataLen);
                load_data(pCmd.addr, pCmd.dataLen, data);
                // send_nl_read_resp(pCmd->addr, data, pCmd->dataLen, pCmd->cmdWord);
                free(data);
                break;
            case PCIE_SOCKET_WRITE_DDR:
                store_data(pCmd.addr, pCmd.dataLen, (const uint8_t*)pCmd.data);
                break;
            default:
                std::cout << "unknow cmd." << std::endl;
            }
            
            lock.lock();
            psp_rv_buf.pop();
            lock.unlock();

        }
    }
    {
        struct cfg_route_table *cfg = nullptr;
        std::unique_lock<std::mutex> lock(cfg_msg_mutex);
        while(!cfg_route_que.empty()) {
            auto cfg = cfg_route_que.front();
            lock.unlock();
            switch (cfg.cmdWord) {
            case SOCKET_CONNET_UPDATE:
                
                break;
            
            default:
                std::cout << "unknow cmd." << std::endl;
            }
            
            lock.lock();
            cfg_route_que.pop();
            lock.unlock();
 
        }
    }
}

int pcie_socket_sim_t::send_data(reg_t addr, size_t length, uint8_t *data, uint8_t target_id){
    struct pcie_socket_packet msg;
    int rv = PCIE_SOCKET_OK;
    int i;
    pcie_socket_packet bsend;
    msg.cmdWord = PCIE_SOCKET_WRITE_DDR;
    msg.addr = addr;
    msg.dataLen = length;
    msg.group_id = board_connect_id;
    msg.board_id = board_id;
    memcpy(&msg.data, data, length);

    for( i = 0; route_table.size(); i++)
    {
        if (route_table[i].emu_board_id == target_id)
            break;
    }
    if (i == route_table.size())
    {
        std::cout << "target npu " << target_id << "not set!" << std::endl;
        return PCIE_SOCKET_ERR;
    }

    rv = sender(route_table[i].server_connect_fd, &msg, sizeof(msg), (struct sockaddr*)&mult_addr, 
        sizeof(mult_addr));
    if (rv != PCIE_SOCKET_OK)
    {
        perror("msg send error, please check network!");
        return rv;
    }

    rv = read(route_table[i].server_connect_fd, (void *)&bsend, sizeof(bsend) - PS_DATA_SIZE_MAX);
    if (bsend.board_id != board_id && bsend.group_id != board_connect_id)
    {
        perror("recv a not match msg!");
    }

    if (bsend.cmdWord != PCIE_SOCKET_OK)
    {
        switch(bsend.cmdWord){
            case PCIE_SOCKET_RECIVE_BOAID_ERR:
                perror("send ip addr or port error!");
                break;
            case PCIE_SOCKET_RECIVE_GROID_ERR:
                perror("send group no match error!");
                break;
            default:
                perror("can't not recongnized cmdword!");
                break;
        }
    }
    return rv;
}

pcie_socket_sim_t::~pcie_socket_sim_t(){
    if (reg_base != nullptr)
        free(reg_base);

    if (sr_t != nullptr){
        if (sr_t->joinable())
            sr_t->join();
    }
}